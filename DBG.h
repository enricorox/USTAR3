//
// Created by enrico on 20/12/22.
//

#ifndef USTAR_DBG_H
#define USTAR_DBG_H

#include <string>
#include <vector>

#define MAX_LINE_LEN 100000

using namespace std;

struct edge_t{
    uint32_t successor;
    bool forward;
    bool to_forward;
};

struct node_t{
    uint32_t length;
    string unitig;
    double avg_abundance;
    vector<int> abundances;
    vector<edge_t> edges;
};

class DBG{
    string bcalm_file_name;
    int kmer_size = 0;
    vector<node_t> nodes;
    size_t n_edges = 0;
    size_t n_kmers = 0;
    double avg_unitig_len = 0;
    /**
     * Parse the BCALM2 file
     * @param bcalm_file_name
     */
    void parse_bcalm_file();

    bool overlaps(const node_t &node, const edge_t &edge);

public:
    /**
     * Construct a de Brujin Graph from a BCALM2 file
     * @param bcalm_file_name
     */
    DBG(const string &bcalm_file_name, int kmer_size);

    ~DBG();

    void print_info();

    bool verify_overlaps();

    void to_bcalm_file(const string &file_name);

    bool validate();

    static string reverse_complement(const string &s);

    string spell(const vector<node_t *> &path_nodes, const vector<edge_t *> &path_edges);

    bool check_path_consistency(const vector<node_t *> &path_nodes, const vector<edge_t *> &path_edges);
};

#endif //USTAR_DBG_H
