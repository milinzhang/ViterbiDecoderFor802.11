/*
 * Copyright (c) 2022 Milin Zhang
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
# ifndef _VITERBI_H_
# define _VITERBI_H_

const int nstates = 64;
const int table_length = 48;
const int traceback_depth = 32;
const int decode_length = table_length - traceback_depth;
const unsigned char branch_table[2][nstates/2] = {{0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,1}, // bit 0 outter branch
                                                  {0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1,0,1,1,0,1,0,0,1}};// bit 1 outter branch

class Viterbi {

    public:

    Viterbi(){};
    ~Viterbi(){};
    void decode(unsigned char*,unsigned char*,int);
    //void depuncture();

    protected:

    unsigned char hamming_distance(unsigned char,unsigned char);
    void branch_distance_compute(unsigned char*,unsigned char*,unsigned char*,unsigned char);
    void add_compare_select(unsigned int*, unsigned char*, unsigned char*, unsigned char*);
    void init_state(unsigned char);
    void find_max_score(unsigned char &);
    void generate_output(unsigned char*,unsigned char,unsigned char&,int);
    void trace_back(unsigned char&, unsigned char &);
    //void depuncture();

    private:

    unsigned char survivor_path[table_length][nstates]; // to store path (previous state)
    unsigned char branch_metric_0[nstates]; // to store hamming distance for outter branch
    unsigned char branch_metric_1[nstates]; // to store hamming distance for inner branch
    unsigned int path_metric[nstates];  // to store cumulative hamming distance for path 
    
};

# endif