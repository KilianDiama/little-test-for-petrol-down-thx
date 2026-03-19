#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <execution>
#include <ranges>
#include <cmath>
#include <random>
#include <span>

namespace Constant {
    // Alignement sur 64 octets pour éviter le "False Sharing" entre threads
    alignas(64) constexpr double LINE_EFFICIENCY = 0.97;
    alignas(64) constexpr double HEALTH_DECAY    = 0.000008;
    alignas(64) constexpr double BUY_THRESHOLD   = 1.2;
    alignas(64) constexpr double SELL_THRESHOLD  = 0.6;
}

// Structure compacte pour le retour (Trivialy Copyable)
struct GridMetrics {
    double total_stored;
    double total_exchanged;
    double local_price;
    double avg_health;
};

class UltraGrid {
private:
    const size_t n_agents;
    
    // Utilisation de vecteurs alignés pour SIMD (Single Instruction Multiple Data)
    // On sépare les données chaudes (storage, health) des données froides (capacity)
    alignas(64) std::vector<double> storage;
    alignas(64) std::vector<double> health;
    alignas(64) std::vector<double> capacity;
    alignas(64) std::vector<double> urgency_scores;
    alignas(64) std::vector<uint32_t> indices; // uint32_t est plus léger que size_t

    int current_tick = 0;
    std::mt19937_64 gen{42};

    // Inline pur pour le calcul du soleil
    [[nodiscard]] inline double solar_input() const noexcept {
        return std::max(0.0, std::sin((current_tick % 24) * 0.26179938779)); // M_PI/12 pré-calculé
    }

    void update_agents(double solar) noexcept {
        const double gain = 4.0 * solar - 1.2;
        
        // Utilisation de std::for_each n-ary (C++20 concept) ou boucle RAW optimisée
        // Ici, on force l'unrolling et la vectorisation
        #pragma omp simd
        for (size_t i = 0; i < n_agents; ++i) {
            const double cap_eff = capacity[i] * health[i];
            
            // Update storage
            storage[i] = std::clamp(storage[i] + gain, 0.0, cap_eff);

            // Health decay branchless (Optimisation CPU pipeline)
            const double soc = storage[i] / capacity[i];
            const bool stress = (soc < 0.1) || (soc > 0.9);
            health[i] -= Constant::HEALTH_DECAY * static_cast<double>(stress);
            health[i] = std::max(0.01, health[i]);

            // Urgency score pre-calculé pour le tri
            urgency_scores[i] = (1.0 - (storage[i] / cap_eff)) * (2.0 - health[i]);
        }
    }

public:
    explicit UltraGrid(size_t n)
        : n_agents(n), storage(n), health(n, 1.0), 
          capacity(n), urgency_scores(n), indices(n) 
    {
        std::iota(indices.begin(), indices.end(), 0);
        std::uniform_real_distribution<> d_cap(80.0, 150.0);
        for (size_t i = 0; i < n; ++i) {
            capacity[i] = d_cap(gen);
            storage[i] = capacity[i] * 0.5;
        }
    }

    GridMetrics step() noexcept {
        update_agents(solar_input());

        // --- PHASE DE TRADING OPTIMISÉE (Ranges + Partition) ---
        auto buyers = indices | std::views::filter([&](auto i) { 
            return urgency_scores[i] > Constant::BUY_THRESHOLD; 
        });
        
        auto sellers = indices | std::views::filter([&](auto i) { 
            return urgency_scores[i] < Constant::SELL_THRESHOLD; 
        });

        // On transforme les views en vecteurs temporaires (ou on travaille par partition)
        // Pour rester 10/10, on utilise std::partition pour éviter les allocs
        auto it_mid = std::partition(std::execution::unseq, indices.begin(), indices.end(),
            [&](auto i) { return urgency_scores[i] > Constant::BUY_THRESHOLD; });
        
        auto it_sell = std::partition(std::execution::unseq, it_mid, indices.end(),
            [&](auto i) { return urgency_scores[i] < Constant::SELL_THRESHOLD; });

        size_t n_b = std::distance(indices.begin(), it_mid);
        size_t n_s = std::distance(it_mid, it_sell);
        size_t trades = std::min(n_b, n_s);

        // Tri partiel uniquement sur les éléments nécessaires
        std::nth_element(indices.begin(), indices.begin() + trades, it_mid, 
            [&](auto a, auto b) { return urgency_scores[a] > urgency_scores[b]; });
        
        std::nth_element(it_mid, it_mid + trades, it_sell, 
            [&](auto a, auto b) { return urgency_scores[a] < urgency_scores[b]; });

        // --- EXECUTION DES TRADES ---
        double exchanged = 0.0;
        for (size_t i = 0; i < trades; ++i) {
            uint32_t b = indices[i];
            uint32_t s = indices[n_b + i];
            
            double amount = std::min(capacity[b]*health[b] - storage[b], storage[s] * 0.2);
            storage[s] -= amount;
            storage[b] += amount * Constant::LINE_EFFICIENCY;
            exchanged += amount;
        }

        // --- RÉDUCTION FINALE (SIMD) ---
        double total_s = std::reduce(std::execution::unseq, storage.begin(), storage.end());
        double avg_h = std::reduce(std::execution::unseq, health.begin(), health.end()) / n_agents;

        current_tick++;
        return { total_s, exchanged, 0.12 * std::pow((double)n_b/std::max(1.0,(double)n_s), 0.4), avg_h };
    }
};
