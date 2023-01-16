#include <chrono>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <functional>
#include <unordered_map>
#include <cmath>
#include <string>

std::ifstream create_is(std::string filename, bool binary = true) {
    if (binary) {
        std::ifstream is(filename, std::ios::binary);
        return is;
    }
    std::ifstream is(filename);
    return is;
}

std::ofstream create_os(std::string filename, bool binary = true) {
    if (binary) {
        std::ofstream os(filename, std::ios::binary);
        return os;
    }
    std::ofstream os(filename);
    return os;
}

template<typename T>
std::istream& raw_read(std::istream& is, T& read_into, size_t size = sizeof(T)) {
    return is.read(reinterpret_cast<char*>(&read_into), size);
}

template<typename T>
std::ostream& raw_write(std::ostream& os, const T& write_to, size_t size = sizeof(T)) {
    return os.write(reinterpret_cast<const char*>(&write_to), size);
}

auto read_samples(const std::string& filename) {
    std::vector<int16_t> samples;
    std::ifstream is(filename, std::ios::binary);
    if (is) {
        int16_t s;
        while(raw_read(is, s)) {
            samples.push_back(s);
        }
    }
    return samples;
}

template<typename T>
class frequencies {
    std::unordered_map<T, int16_t> counters_;
    uint32_t tot_ = 0;

public:
    void operator()(const T& val) {
        ++counters_[val];
        ++tot_;
    }

    uint32_t tot() { return tot_; }
    auto begin() { return counters_.begin(); }
	auto begin() const { return counters_.begin(); }
	auto end() { return counters_.end(); }
	auto end() const { return counters_.end(); }

    const double entropy() {
        double H = 0.0;
        for (const auto& x : counters_) {
			double px = x.second;
			H += px * log2(px);
		}
		return log2(tot_) - H / tot_;
    }
};

template<typename T>
auto quantize(std::vector<T> original, int Q) {
    std::vector<T> quantized;
    for (const auto& x : original) {
        quantized.push_back(x / Q);
    }

    return quantized;
}

template<typename T>
auto dequantize(std::vector<T> quantized, int Q) {
    std::vector<T> dequantized;
    for (const auto& x : quantized) {
        dequantized.push_back(x * Q);
    }
    
    return dequantized;
}

template<typename T>
auto qt_error(std::vector<T> original, std::vector<T> dequantized) {
    std::vector<T> qt_error;
    for (int i = 0; i < original.size(); ++i) {
        qt_error.push_back(original[i] - dequantized[i]);
    }
    
    return qt_error;
}

class MDCT {
    int N_;
    std::vector<double> w_;
    std::vector<std::vector<double>> c_;

public:
    MDCT(int N) : N_(N), w_(2 * N), c_(N, std::vector<double>(2 * N)) {
        for (size_t n = 0; n < 2 * N; ++n) {
            w_[n] = std::sin(M_PI / (2 * N) * (n + 0.5));
        }

        for (size_t k = 0; k < N; ++k) {
            for (size_t n = 0; n < 2 * N; ++n) {
                c_[k][n] = std::cos(M_PI / N * (n + 0.5 + N * 0.5) * (k + 0.5));
            }
        }
    }

    void forward(std::vector<double>& X, std::vector<int16_t>& x, int w_index) {
        for (size_t k = 0; k < N_; ++k) {
            double X_k = 0.0;
            auto& c_k = c_[k]; // This is a vector
            for (size_t n = 0; n < N_ * 2; ++n) {
                int16_t x_n = x[w_index * N_ + n];
                double w_n = w_[n];
                double c = c_k[n];

                X_k += x_n * w_n * c;
            }
            X[w_index * N_ + k] = X_k;
        }
    }

    void backward(std::vector<double>& X, std::vector<double>& y, int w_index) {
        for (size_t n = 0; n < 2 * N_; ++n) {
            double y_n = 0.0;
            for (size_t k = 0; k < N_; ++k) {
                double X_k = X[w_index * N_ + k];
                double c = c_[k][n];

                y_n += X_k * c;
            }
            double w_n = w_[n];
            y[n] = 2.0 / N_ * w_n * y_n;
        }
    }
};

int main(int argc, char *argv[]) {
    // Program setup
    const std::string filename = argv[1];
    int Q = 2600;
    bool QUANTIZE = false;
    bool ENTROPY = false;
    bool TRANSFORM = true;

    // Start running time
    std::chrono::steady_clock::time_point _start(std::chrono::steady_clock::now());

    // Read samples from audio file
    std::cout << "Reading samples...\n";
    auto samples = read_samples(filename);
    std::cout << "Done! " << samples.size() << " samples acquired\n";

    // Quantize sampled values
    auto quantized = quantize(samples, Q);

    if (QUANTIZE) {
        // Reconstruct the vector
        auto dequantized = dequantize(quantized, Q);
        auto output_qt = create_os("output_qt.raw");
        for (const auto& x : dequantized) { raw_write(output_qt, x); }
    
        // Compute the error
        auto error = qt_error(samples, dequantized);
        auto error_qt = create_os("error_qt.raw");
        for (const auto& x : error) { raw_write(error_qt, x); }
    }
    if (ENTROPY) {
        // Computing original entropy
        std::cout << "Computing original entropy...\n";
        auto f_original = std::for_each(samples.begin(), samples.end(), frequencies<int16_t>());
        std::cout << "Done! Entropy: " << f_original.entropy();

        // Computing quantized entropy
        std::cout << "\nComputing quantized entropy...\n";
        auto f_quantized = std::for_each(quantized.begin(), quantized.end(), frequencies<int16_t>());
        std::cout << "Done! Entropy: " << f_quantized.entropy();
    }
    if (TRANSFORM) {
        // MDCT Params
        int N = 1024;
        int W = 2 * N;
        // How many windows will I have?
        int w_number = (samples.size() + N - 1) / N;
        // Initializing padded vector with all 0s
        std::vector<int16_t> padded((w_number + 2) * N);
        // Copying samples into the new vector leaving 1024/2 0 at front and at the end
        copy(begin(samples), end(samples), next(begin(padded), N));

        // Creating the MDCT object and computing sin and cos
        MDCT mdct(N);
        // Applying the MDCT forward pass
        std::vector<double> X((w_number + 1) * N);
        for(int i = 0; i < w_number + 1; ++i) {
            mdct.forward(X, padded, i);
        }

        // Quantize values
        double Qt = 10000.0;
        auto X_qt = quantize(X, Qt);
        auto f_X_qt = std::for_each(X_qt.begin(), X_qt.end(), frequencies<int16_t>());

        // Dequantize values
        auto X_dqt = dequantize(X_qt, Qt);

        // Applying IMDCT
        std::vector<int16_t> transr(w_number * N);
        std::vector<double> prev_inv(2 * N);
        std::vector<double> curr_inv(2 * N);

        mdct.backward(X_dqt, prev_inv, 0);
        for (int i = 1; i < w_number + 1; ++i) {
            mdct.backward(X_dqt, curr_inv, i);

            // Adding the 2 half together
            std::transform(
                next(begin(prev_inv), N),
                end(prev_inv),
                begin(curr_inv),
                next(begin(transr ), N * (i - 1)),
                [](double a, double b){ return static_cast<int16_t>(a + b); }
            );

            prev_inv = curr_inv;
        }
        
    }

    // Stop running time and print result
    std::chrono::steady_clock::time_point _stop(std::chrono::steady_clock::now());
    std::cout << "\nElapsed time: " << 
        std::chrono::duration_cast<std::chrono::duration<double>>(_stop - _start).count();
}