//
// Created by enrico on 22/12/22.
//

#ifndef USTAR_ENCODER_H
#define USTAR_ENCODER_H

#include <vector>
#include <map>
#include "consts.h"
#include "ColorGraph.h"

using namespace std;

class Encoder{
    bool debug = true;
    encoding_t encoding{};
    bool encoding_done = false;

    vector<bool> flips;
    vector<size_t> simplitigs_order;
    vector<double> avg_counts;

    const vector<string> *simplitigs;
    const vector<vector<uint32_t>> *simplitigs_colors;
    vector<string> simplitigs_aux;
    vector<vector<uint32_t>> simplitigs_colors_aux;
    size_t n_kmers = 0;

    vector<uint32_t> symbols;
    vector<uint32_t> runs;
    double avg_run = 0;

    vector<uint32_t> compacted_counts;

    long bwt_primary_index = 0;

    ColorGraph *cg;

    void do_RLE();

    void compute_avg();

    void do_flip();

    void compact_counts();

public:
    Encoder(const vector<string> *simplitigs, const vector<vector<uint32_t>> *simplitigs_colors, bool debug=false);

    void encode(encoding_t encoding_type);

    void to_fasta_file(const string &file_name);

    void to_counts_file(const string &file_name);

    void print_stat();

    void to_colors_file(const string &file_name);
};
#endif //USTAR_ENCODER_H
