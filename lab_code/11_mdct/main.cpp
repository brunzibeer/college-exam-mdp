#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <iterator>

#define _USE_MATH_DEFINES
#include <math.h>

#include "rawio.h"

auto read_samples(const std::string& filename)
{
    std::vector<int16_t> samples;
    std::ifstream is(filename, std::ios::binary);
    if (is) {
        int16_t s;
        while (raw_read(is, s)) {
            samples.push_back(s);
        }
    }
    return samples;
}

void write_samples(const std::string& filename, const std::vector<int16_t>& samples)
{
    std::ofstream os(filename, std::ios::binary);
    for (const int16_t& x : samples) {
        raw_write(os, x);
    }
}

template <typename T>
class freq {
    std::unordered_map<T, uint32_t> counters_;
    uint32_t tot_ = 0;
public:
    void operator()(const T& val) {
        ++counters_[val];
        ++tot_;
    }

    uint32_t tot() const { return tot_; }

    auto begin() { return counters_.begin(); }
    auto begin() const { return counters_.begin(); }
    auto end() { return counters_.end(); }
    auto end() const { return counters_.end(); }

    double entropy()
    {
        double h = 0;
        for (const auto& x : counters_) {
            double p = x.second;
            h += p * log2(p);
        }
        return log2(tot_) - h / tot_;
    }
};

template <typename T>
auto quantize(const std::vector<T>& samples, double Q)
{
    std::vector<int16_t> quantized(samples.size());
    std::transform(begin(samples), end(samples), begin(quantized),
        [&Q](T s) {
            //return static_cast<int16_t>(s / Q); // Rounds towards 0 always.
            return static_cast<int16_t>(std::round(s / Q)); // Round towards the nearest integer
        }
    );
    return quantized;
}

template <typename T>
auto dequantize(const std::vector<int16_t>& quantized, double Q)
{
    std::vector<T> reconstructed(quantized.size());
    std::transform(begin(quantized), end(quantized), begin(reconstructed),
        [&Q](int16_t s) { return static_cast<T>(s * Q); }
    );
    return reconstructed;
}

auto compute_error(const std::vector<int16_t>& samples, const std::vector<int16_t>& reconstructed)
{
    std::vector<int16_t> error(samples.size());
    std::transform(begin(samples), end(samples), begin(reconstructed),
        begin(error), [](int16_t s, int16_t r) { return s - r; }
    );
    return error;
}

class MDCT {
    int N;
    std::vector<double> wt;
    std::vector<std::vector<double>> ct;
public:

    MDCT(int _N) : N(_N), wt(2 * _N), ct(N, std::vector<double>(2 * _N))
    {
        for (int n = 0; n < 2 * N; ++n) {
            wt[n] = sin(M_PI / (2 * N) * (n + 0.5));
        }

        for (int k = 0; k < N; ++k) {
            for (int n = 0; n < 2 * N; ++n) {
                ct[k][n] = cos(M_PI / N * (n + 0.5 + N * 0.5) * (k + 0.5));
            }
        }
    }

    void forward(const std::vector<int16_t>& x, std::vector<double>& X, int window_index)
    {
        for (int k = 0; k < N; ++k) {
            double Xk = 0.0;
            auto& ctk = ct[k];
            for (int n = 0; n < 2 * N; ++n) {
                int16_t xn = x[window_index*N + n];
                double wn = wt[n];
                double c = ctk[n];

                Xk += xn * wn * c;
            }
            X[window_index*N + k] = Xk;
        }
    }

    void backward(const std::vector<double>& X, std::vector<double>& y, int window_index)
    {
        for (int n = 0; n < 2 * N; ++n) {
            double yn = 0.0;
            for (int k = 0; k < N; ++k) {
                double Xk = X[window_index*N + k];
                double c = ct[k][n];

                yn += Xk * c;
            }
            double wn = wt[n];
            y[n] = 2.0 / N * wn * yn;
        }
    }
};

int main(int argc, char *argv[])
{
    {
        using clock = std::chrono::steady_clock;
        std::cout << "Start...\n";
        auto start_time = clock::now();

        std::cout << "Reading... ";
        auto samples = read_samples("test.raw");
        std::cout << "done.\n";
        std::cout << "Number of samples: " << samples.size() << "\n";

        std::cout << "Computing frequencies... ";
        auto f = for_each(begin(samples), end(samples), freq<int16_t>());
        std::cout << "done.\n";
        std::cout << "f.entropy(): " << f.entropy() << "\n";

        if (true) {
            // Quantize
            int Q = 2600;
            std::cout << "Quantizing... ";
            auto quantized = quantize(samples, Q);
            std::cout << "done.\n";

            std::cout << "Computing frequencies... ";
            auto fq = for_each(begin(quantized), end(quantized), freq<int16_t>());
            std::cout << "done.\n";
            std::cout << "fq.entropy(): " << fq.entropy() << "\n";

            // Reconstruct
            std::cout << "Quantizing... ";
            auto reconstructed = dequantize<int16_t>(quantized, Q);
            std::cout << "done.\n";
            std::cout << "Saving reconstructed signal... ";
            write_samples("reconstructed.raw", reconstructed);
            std::cout << "done.\n";

            // Error
            std::cout << "Computing error... ";
            auto error = compute_error(samples, reconstructed);
            std::cout << "done.\n";
            std::cout << "Saving error signal... ";
            write_samples("error.raw", error);
            std::cout << "done.\n";
        }

        if (true) {
            // MDCT
            std::cout << "Copying and padding samples... ";
            int N = 1024;
            int W = 2 * N; // Window size
            int num_win = (samples.size() + N - 1) / N;
            std::vector<int16_t> padded((num_win + 2)*N); 
            copy(begin(samples), end(samples), next(begin(padded), N));
            std::cout << "done.\n";

            std::cout << "Precomputing sines and cosines... ";
            MDCT mdct(N);
            std::cout << "done.\n";

            std::cout << "Computing MDCT... ";
            std::vector<double> trans((num_win + 1)*N);
            for (int i = 0; i < num_win + 1; ++i) {
                mdct.forward(padded, trans, i);
            }
            std::cout << "done.\n";

            // Quantize
            std::cout << "Quantizing... ";
            double Q = 10000.;
            auto transq = quantize(trans, Q);
            std::cout << "done.\n";
            std::cout << "Computing frequencies... ";
            auto ftq = for_each(begin(transq), end(transq), freq<int16_t>());
            std::cout << "done.\n";
            std::cout << "ftq.entropy(): " << ftq.entropy() << "\n";

            // Reconstruct
            std::cout << "Dequantizing... ";
            auto transdq = dequantize<double>(transq, Q);
            std::cout << "done.\n";

            // IMDCT
            std::cout << "Computing IMDCT... ";
            std::vector<int16_t> transr(num_win*N);
            std::vector<double> prev_inv(2 * N);
            std::vector<double> curr_inv(2 * N);

            mdct.backward(transdq, prev_inv, 0);
            for (int i = 1; i < num_win + 1; ++i) {
                // Inverse MDCT
                mdct.backward(transdq, curr_inv, i);

                // Add 2nd half of prev to 1st half of curr
                std::transform(
                    next(begin(prev_inv), N), end(prev_inv), // First operand From, To
                    begin(curr_inv),                         // Second operand From (To is deduced from First)
                    next(begin(transr), N * (i - 1)),        // Output From (To is deduced from First)
                    [](double a, double b) { return static_cast<int16_t>(a + b); } // Predicate
                );

                // prev <- curr
                prev_inv = curr_inv;
            }
            std::cout << "done.\n";

            std::cout << "Saving reconstructed signal... ";
            write_samples("transr.raw", transr);
            std::cout << "done.\n";

            // Error
            std::cout << "Computing error... ";
            auto error = compute_error(samples, transr);
            std::cout << "done.\n";
            std::cout << "Saving error signal... ";
            write_samples("error_mdct.raw", error);
            std::cout << "done.\n";
        }

        auto stop_time = clock::now();
        std::cout << "Stop.\n";
        auto diff = stop_time - start_time;
        std::cout << "Elapsed: " << diff.count() / 1000000000.0 << " s\n";
    }
	return EXIT_SUCCESS;
}