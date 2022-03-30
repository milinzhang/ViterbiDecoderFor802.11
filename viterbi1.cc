# include "viterbi1.h"
# include <stdio.h>
unsigned char Viterbi::hamming_distance(unsigned char received_bit, unsigned char encoded_bit){
    
    unsigned char res;
    if (received_bit==2) { // when the received bit is a dummy bit
        res = 0;
    } else if (received_bit == encoded_bit) {
        res = 0;
    } else {
        res = 1;
    }
    return res;
}

/*-------------------------------- forward step ----------------------------------------------*/

void Viterbi::branch_distance_compute (unsigned char *received_bits, unsigned char *branch_metric_0, unsigned char *branch_metric_1, unsigned char t) {

    for (int i=0; i<nstates/2; i++) {
        // to compute outter branch
        branch_metric_0[i] = 2 - hamming_distance(received_bits[2*t],branch_table[0][i])
                               - hamming_distance(received_bits[2*t+1],branch_table[1][i]);

        branch_metric_0[i+nstates/2] = 2 - hamming_distance(received_bits[2*t],branch_table[0][i]);
                                         - hamming_distance(received_bits[2*t+1],branch_table[1][i]);

        // to compute inner branch
        branch_metric_1[i] = hamming_distance(received_bits[2*t],branch_table[0][i])
                           + hamming_distance(received_bits[2*t+1],branch_table[0][i]);

        branch_metric_1[i+nstates/2] = hamming_distance(received_bits[2*t],branch_table[0][i])
                                     + hamming_distance(received_bits[2*t+1],branch_table[1][i]);
    }
}

void Viterbi::add_compare_select(unsigned int *path_metric, unsigned char *branch_0, unsigned char *branch_1, unsigned char *sur_path) {

    unsigned int path_metric_temp_0[nstates], path_metric_temp_1[nstates];

    for (int i=0; i<nstates/2; i++) {
        // add
        path_metric_temp_0[i*2] = path_metric[i] + branch_0[i];
        path_metric_temp_0[i*2+1] = path_metric[i+nstates/2] + branch_0[i+nstates/2];
        path_metric_temp_1[i*2] = path_metric[i+nstates/2] + branch_1[i+nstates/2];
        path_metric_temp_1[i*2+1] = path_metric[i] + branch_1[i];

    }

    for (int i=0; i<nstates/2; i++) {
        // compare and select
        if (path_metric_temp_0[i*2]>path_metric_temp_1[i*2]) { // when the outter branch is larger 
            path_metric[2*i] = path_metric_temp_0[i*2];
            sur_path[2*i] = i;
        } else {    // when the inner branch is larger
            path_metric[2*i] = path_metric_temp_1[i*2];
            sur_path[2*i] = i+nstates/2;
        }

        if (path_metric_temp_0[i*2+1]>path_metric_temp_1[2*i+1]) { // when the outter branch is larger
            path_metric[2*i+1] = path_metric_temp_0[i*2+1];
            sur_path[2*i+1] = i + nstates/2;
        } else {    // when the inner branch is larger
            path_metric[2*i+1] = path_metric_temp_1[i*2+1];
            sur_path[2*i+1] = i;
        }
    }
}
/*------------------------------ fix initial state ----------------------------------*/

void Viterbi::init_state (unsigned char istate) {

    for (int i=0; i<nstates; i++) {
        branch_metric_0[i] = 0;
        branch_metric_1[i] = 0;
        path_metric[i] = 0;
    }
    for (int j=0; j<table_length; j++) {
        for (int k=0; k<nstates; k++) {
            survivor_path[j][k] = 0;
        }
    }
    path_metric[istate] = 13;
}
/*-------------------------------- backward step -----------------------------------------------*/
void Viterbi::find_max_score(unsigned char &beststate) {

    unsigned int bestbranch = path_metric[0];
    beststate = 0;
    for (int i=1; i<64; i++) {
        if (bestbranch<path_metric[i]) {
            bestbranch = path_metric[i];
            beststate = i;
        }
    }
    printf("%d\n",bestbranch);
}

void Viterbi::generate_output (unsigned char *output, unsigned char cur_state, unsigned char &pos, int output_length) {

    for (int i=0; i<output_length; i++) {
        output[output_length-i-1] = (cur_state%2)?1:0;
        cur_state = survivor_path[(pos+table_length-i-1)%table_length][cur_state];
    }

    pos = (pos+output_length)%table_length; // update the position in circular table
}

void Viterbi::trace_back (unsigned char &pos, unsigned char &current_state) {

    for (int t=0; t<traceback_depth; t++) {
        current_state = survivor_path[(pos-t-1+table_length)%table_length][current_state];
    }

    pos = (pos+decode_length)%table_length;
}
/*
void depuncture () {

}*/

/*--------------------------- viterbi decoder ---------------------------------*/

void Viterbi::decode (unsigned char *received_bits, unsigned char *decoded_bits, int num_of_decoded_bits) {

    // initialization
    unsigned char initial_state = 0;
    init_state(initial_state);

    int i = 0; // to track the num of processed input step; note: every step we process 2 bits
    int o = 0; // to track the num of processed output
    unsigned char position = 0; // to track the position in circular table
    unsigned char current_state; // to record the best branch


    while(i<num_of_decoded_bits) {

        if (num_of_decoded_bits-i<=decode_length) { // when the rest of input can be fully stored in the circular table, decode
            for (int t=0; t<num_of_decoded_bits-i; t++) {
                branch_distance_compute(received_bits+i*2,branch_metric_0,branch_metric_1,t);
                add_compare_select(path_metric,branch_metric_0,branch_metric_1,survivor_path[position]);
                position = (position+1)%table_length;
            }
            i = num_of_decoded_bits;
            find_max_score(current_state);
            generate_output(decoded_bits+o,current_state,position,num_of_decoded_bits-o);

        } else {  // when the input can not fully processed by the length of circular table, process 16 steps per time
            for (int t=0; t<decode_length; t++) { // forward phase
                branch_distance_compute(received_bits+i*2,branch_metric_0,branch_metric_1,t);
                add_compare_select(path_metric,branch_metric_0,branch_metric_1,survivor_path[position]);
                position = (position+1)%table_length;
            }
            i+=decode_length;
            if ((i-table_length)>=0 && (i-table_length)%(decode_length)==0) { // backward phase
                find_max_score(current_state);
                trace_back(position,current_state);
                generate_output(decoded_bits+o,current_state,position,decode_length);
                o+=decode_length;
            }
        }
    }
}