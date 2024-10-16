//
// Created by enrico on 22/12/22.
//

#include <fstream>
#include <iostream>
#include <algorithm>

#include "Encoder.h"
#include "DBG.h"
#include "commons.h"
#include "bwt.hpp"
#include "SPSS.h"
#include "ColorGraph.h"

Encoder::Encoder(const vector<string> *simplitigs, const vector<vector<uint32_t>> *simplitigs_colors, bool debug) {
    if (simplitigs_colors->empty()) {
        cerr << "Encoder(): There are no simplitigs_colors!" << endl;
        exit(EXIT_FAILURE);
    }
    if(simplitigs->size() != simplitigs_colors->size()){
        cerr << "Encoder(): Got an incorrect number of simplitigs_colors or simplitigs!" << endl;
        exit(EXIT_FAILURE);
    }

    this->debug = debug;
    this->simplitigs = simplitigs;
    this->simplitigs_colors = simplitigs_colors;

    simplitigs_order.reserve(simplitigs->size());
    for(size_t i = 0; i < simplitigs->size(); i++)
        simplitigs_order.push_back(i);

    flips.resize(simplitigs_colors->size(), false);

    for(auto &counts : *simplitigs_colors)
        n_kmers += counts.size();
}

void Encoder::to_fasta_file(const string &file_name) {
    if(encoding == encoding_t::OPT_RLE) {
        cg->write_sequences(file_name);
        return;
    }
    if(simplitigs->empty()){
        cerr << "to_fasta_file(): There are no simplitigs!" << endl;
        exit(EXIT_FAILURE);
    }
    if(debug){
        if(simplitigs_order.size() != simplitigs->size()) {
            cerr << "Order vector must be the same size!" << endl;
            exit(EXIT_FAILURE);
        }
    }
    ofstream fasta;
    fasta.open(file_name);
    for(size_t i = 0; i < simplitigs->size(); i++){
        auto &simplitig = (*simplitigs)[simplitigs_order[i]];
        fasta << ">\n";
        fasta << (flips[i]?DBG::reverse_complement(simplitig):simplitig) << "\n";
    }
    fasta.close();
    cout << "Done!" << endl;
}

void Encoder::to_counts_file(const string &file_name) {
    ofstream encoded;
    encoded.open(file_name);
    if(!encoded.good()){
        cerr << "Can't open output file: " << file_name + ".rle" << endl;
        exit(EXIT_FAILURE);
    }

    switch(encoding) {
        case encoding_t::AVG_FLIP_RLE:
            // no break here
        case encoding_t::FLIP_RLE:
            // no break here
        case encoding_t::AVG_RLE:
            // no break here
        case encoding_t::RLE:
            for(size_t i = 0; i < symbols.size(); i++){
                encoded << symbols[i];
                if(runs[i] != 1)
                    encoded << RLE_SEPARATOR << runs[i];
                encoded << "\n";
            }
            encoded.close();
            break;
        case encoding_t::BWT:
            encoded << bwt_primary_index << "\n";
            for(auto c : compacted_counts)
                encoded << c << "\n";
            break;
        case encoding_t::FLIP:
            // no break here
        case encoding_t::PLAIN:
            for(auto i : simplitigs_order){
                auto &simplitig = (*simplitigs_colors)[i];
                for(size_t j = 0; j < simplitig.size(); j++){
                    size_t curr = simplitig[j];
                    if(flips[i])
                        curr = simplitig[simplitig.size() - 1 - j];
                    encoded << curr << (debug ? " " : "\n");
                }
                if (debug) encoded << "\n";
            }
            break;
        case encoding_t::BINARY:
            for(auto i : simplitigs_order){
                auto &simplitig = (*simplitigs_colors)[i];
                for(size_t j = 0; j < simplitig.size(); j++){
                    size_t curr = simplitig[j];
                    if(flips[i])
                        curr = simplitig[simplitig.size() - 1 - j];
                    if(curr > UINT32_MAX){
                        cerr << "to_counts_file(): count is too large for UINT16: " << curr << " > " << UINT32_MAX << endl;
                        exit(EXIT_FAILURE);
                    }
                    uint32_t c = curr;
                    //encoded.write((char*) &curr, sizeof(curr));
                    encoded.write((char*) &c, sizeof(c));
                }
            }
            break;
        default:
            cerr << "to_counts_file(): Unknown encoding" << endl;
            exit(EXIT_FAILURE);
    }
    encoded.close();
}

void Encoder::to_colors_file(const string &file_name) {
    if(encoding == encoding_t::OPT_RLE) {
        cg->write_colors(file_name);
        return;
    }

    ofstream encoded;
    encoded.open(file_name);
    if(!encoded.good()){
        cerr << "Can't open output file: " << file_name + ".rle" << endl;
        exit(EXIT_FAILURE);
    }

    switch(encoding) {
        case encoding_t::AVG_FLIP_RLE:
            // no break here
        case encoding_t::FLIP_RLE:
            // no break here
        case encoding_t::AVG_RLE:
            // no break here
        case encoding_t::RLE:
            for(size_t i = 0; i < symbols.size(); i++){
                encoded << symbols[i];
                if(runs[i] != 1)
                    encoded << RLE_SEPARATOR << runs[i];
                encoded << "\n";
            }
            encoded.close();
            break;
        case encoding_t::BWT:
            encoded << bwt_primary_index << "\n";
            for(auto c : compacted_counts)
                encoded << c << "\n";
            break;
        case encoding_t::FLIP:
            // no break here
        case encoding_t::PLAIN:
            for(auto i : simplitigs_order){
                auto &simplitig = (*simplitigs_colors)[i];
                for(size_t j = 0; j < simplitig.size(); j++){
                    size_t curr = simplitig[j];
                    if(flips[i])
                        curr = simplitig[simplitig.size() - 1 - j];
                    encoded << curr << (debug ? " " : "\n");
                }
                if (debug) encoded << "\n";
            }
            break;
        case encoding_t::BINARY:
            for(auto i : simplitigs_order){
                auto &simplitig = (*simplitigs_colors)[i];
                for(size_t j = 0; j < simplitig.size(); j++){
                    size_t curr = simplitig[j];
                    if(flips[i])
                        curr = simplitig[simplitig.size() - 1 - j];
                    if(curr > UINT32_MAX){
                        cerr << "to_counts_file(): count is too large for UINT16: " << curr << " > " << UINT32_MAX << endl;
                        exit(EXIT_FAILURE);
                    }
                    uint32_t c = curr;
                    //encoded.write((char*) &curr, sizeof(curr));
                    encoded.write((char*) &c, sizeof(c));
                }
            }
            break;
        default:
            cerr << "to_counts_file(): Unknown encoding" << endl;
            exit(EXIT_FAILURE);
    }
    encoded.close();
}

void Encoder::encode(encoding_t encoding_type) {
    this->encoding = encoding_type;
    encoding_done = true;

    switch(encoding) {
            case encoding_t::BWT: {
                compute_avg();
                sort(simplitigs_order.begin(), simplitigs_order.end(),
                     [this](size_t a, size_t b) { return avg_counts[a] < avg_counts[b]; }
                );
                do_flip();
                compact_counts();
                if(debug)
                    cout << "BWT transform a vector of size " << compacted_counts.size() << endl;
                auto key = townsend::algorithm::bwtEncode(compacted_counts.begin(), compacted_counts.end());
                bwt_primary_index = key - compacted_counts.begin();
            }
            break;
        case encoding_t::BINARY:
            // no break here
        case encoding_t::AVG_FLIP_RLE:
            compute_avg();
            sort(simplitigs_order.begin(), simplitigs_order.end(),
                 [this](size_t a, size_t b){return avg_counts[a] < avg_counts[b];}
            );
            do_flip();
            do_RLE();
            break;
        case encoding_t::OPT_RLE: {
            cg = new ColorGraph(*simplitigs, *simplitigs_colors);
            }
            break;
        case encoding_t::FLIP_RLE:
            do_flip();
            do_RLE();
            break;
        case encoding_t::RLE:
            do_RLE();
            break;
        case encoding_t::AVG_RLE:
            compute_avg();
            sort(simplitigs_order.begin(), simplitigs_order.end(),
                 [this](size_t a, size_t b){return avg_counts[a] < avg_counts[b];}
                 );
            do_RLE();
            break;
        case encoding_t::FLIP:
            do_flip();
            break;
        case encoding_t::PLAIN:
            // do nothing
            break;
        default:
            cerr << "encode(): Unknown encoding" << endl;
            exit(EXIT_FAILURE);
    }
}

void Encoder::do_flip(){
    flips[0] = false;
    for(size_t i = 1; i < simplitigs_colors->size(); i++){
        uint32_t prev_last = (*simplitigs_colors)[i - 1].back();
        uint32_t curr_first = (*simplitigs_colors)[i].front();
        uint32_t curr_last = (*simplitigs_colors)[i].back();
        if(d(prev_last, curr_first) == 0) // that's ok
            continue;
        if(d(prev_last, curr_last) == 0) // we can do better!
            flips[i] = true;
    }
}

void Encoder::do_RLE(){
    uint32_t count = 0, prev;
    uint32_t sum_run = 0;

    bool first = true;
    for (size_t i = 0; i < simplitigs_colors->size(); i++) {
        // accessing simplitigs_colors based on computed order!
        auto &counts = (*simplitigs_colors)[simplitigs_order[i]];
        for (size_t k = 0; k < counts.size(); k++) {
            uint32_t curr;
            if(!flips[i]) // forward visiting
                curr = counts[k];
            else // backward visiting
                curr = counts[counts.size() - 1 - k];

            // stream of simplitigs_colors here
            if (first || curr == prev) {
                count++;
                first = false;
            } else {
                symbols.push_back(prev);
                runs.push_back(count);
                sum_run += count;
                // reset
                count = 1;
            }
            prev = curr;
        }
    }
    // save last parse_file
    symbols.push_back(prev);
    runs.push_back(count);
    sum_run += count;

    avg_run = (double) sum_run / (double) runs.size();

    if(debug){
        // sum_run must be equal to n_kmers!!!
        if(sum_run != n_kmers){
            if(sum_run > n_kmers)
                cerr << "OOPS! We have too many colors!" << endl;
            else
                cerr << "OOPS! We have too less colors!" << endl;
            exit(EXIT_FAILURE);
        }else
            cout << "YES! Counts number is correct!\n";
    }
}

void Encoder::compute_avg() {
    // pre-allocate the vector
    avg_counts.reserve(simplitigs_colors->size());

    for(auto &simplitig_colors : *simplitigs_colors){
        if(encoding == encoding_t::AVG_RLE) {
            double sum = 0;
            for (uint32_t c: simplitig_colors)
                sum += c;
            avg_counts.push_back(sum / (double) simplitig_colors.size());
        } else
            avg_counts.push_back((double) (simplitig_colors.front() + simplitig_colors.back()) / 2);
    }

}

void Encoder::print_stat(){
    if(!encoding_done){
        cerr << "print_stats(): Need to encode() first!" << endl;
        exit(EXIT_FAILURE);
    }
    cout << "\nEncoding stats:\n";
    switch (encoding) {
        case encoding_t::OPT_RLE:
            cout << "   Number of runs: " << cg->get_num_run() << "\n";
            cout << "   Average run:    " << cg->get_average_run() << "\n";
            break;
        case encoding_t::BINARY:
            // no break here
        case encoding_t::AVG_FLIP_RLE:
            // no break here
        case encoding_t::FLIP_RLE:
            // no break here
        case encoding_t::AVG_RLE:
            // no break here
        case encoding_t::RLE:
            cout << "   Number of runs: " << runs.size() << "\n";
            cout << "   Average run:    " << avg_run << "\n";
            break;
        case encoding_t::BWT:
            // no break here
        case encoding_t::FLIP:
            // no break here
        case encoding_t::PLAIN:
            cout << "   Number of colors: " << n_kmers << "\n";
            break;
        default:
            cerr << "to_counts_file(): Unknown encoding" << endl;
            exit(EXIT_FAILURE);
    }
    cout << endl;
}

void Encoder::compact_counts() {
    compacted_counts.reserve(n_kmers);
    for(auto i : simplitigs_order){
        auto &simplitig = (*simplitigs_colors)[i];
        for(size_t j = 0; j < simplitig.size(); j++){
            size_t curr = simplitig[j];
            if(flips[i])
                curr = simplitig[simplitig.size() - 1 - j];
            compacted_counts.push_back(curr);
        }
    }
}
