# include "viterbi1.h"

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

    survivor_path[table_length][nstates] = {0};
    branch_metric_0[nstates] = {0};
    branch_metric_1[nstates] = {0};
    path_metric[nstates] = {0};
    path_metric[istate] = 13;
}
/*-------------------------------- backward step -----------------------------------------------*/
void Viterbi::find_max_score(unsigned int &bestbranch, unsigned char &beststate) {

    bestbranch = path_metric[0];
    beststate = 0;
    for (int i=1; i<64; i++) {
        if (bestbranch<path_metric[i]) {
            bestbranch = path_metric[i];
            beststate = i;
        }
    }
}

void Viterbi::generate_output (unsigned char *output, unsigned char cur_state, unsigned char pos, int decode_path_depth) {

    for (int i=0; i<decode_path_depth; i++) {
        output[decode_path_depth-i-1] = (cur_state%2==1)?1:0;
        cur_state = survivor_path[(pos+table_length-i-1)%table_length][cur_state];
    }
}

void Viterbi::trace_back (int index_of_input, unsigned char &position, unsigned char &current_state) {

    for (int t=0; t<traceback_depth; t++) {
        position = (index_of_input-t)%table_length;
        current_state = survivor_path[position][current_state];
    }
}
/*
void depuncture () {

}*/

/*--------------------------- viterbi decoder ---------------------------------*/

void Viterbi::decode (unsigned char *received_bits, unsigned char *decoded_bits, int ninput) {

    // initialization
    unsigned char initial_state = 0;
    init_state(initial_state);

    // when the length of input is less than the table length, we can store all results in table and decode directly
    if (ninput <= table_length) {
        // forward phase
        for (int t=0; t<ninput; t++) {
            branch_distance_compute(received_bits,branch_metric_0,branch_metric_1,t);
            add_compare_select(path_metric,branch_metric_0,branch_metric_1,survivor_path[t]);
        }
        // search for the maximum score
        unsigned int bestbranch;
        unsigned char current_state;
        find_max_score(bestbranch,current_state);
        // decode
        generate_output(decoded_bits,current_state,ninput,ninput);
    }
    // when the num of input is larger than the table length, loop
    else {
        int i = 0; // to track the num of processed input
        int o = 0; // to track the num of processed output
        unsigned char position = 0; // to track the position in circular table

        while(i<ninput) {

            if (ninput-i<table_length-traceback_depth) { // when the rest of input can be fully stored in the circular table, decode
                for (int t=0; t<ninput-i; t++) {
                    branch_distance_compute(received_bits+i,branch_metric_0,branch_metric_1,position+t);
                    add_compare_select(path_metric,branch_metric_0,branch_metric_1,survivor_path[position+t]);
                }
                position = (position+ninput-i)%table_length;
                // search for the maximum score and decode directly
                unsigned int bestbranch;
                unsigned char current_state;
                find_max_score(bestbranch,current_state);
                generate_output(decoded_bits+o,current_state,position,ninput-i);
            }

            else {  // when the input can not fully processed by the length of circular table, process every 16 step (tablelength-tracebackdepth)
                for (int t=0; t<table_length-traceback_depth; t++) {
                    branch_distance_compute(received_bits+i,branch_metric_0,branch_metric_1,position+t);
                    add_compare_select(path_metric,branch_metric_0,branch_metric_1,survivor_path[position+t]);
                }

                if (i-table_length>=0 && (i-table_length)%(table_length-traceback_depth) == 0) {
                    position = (position+ninput-i)%table_length;
                    unsigned int bestbranch;
                    unsigned char current_state;
                    find_max_score(bestbranch,current_state);
                    trace_back(traceback_depth,position,current_state);
                    generate_output(decoded_bits+o,current_state,position,table_length-traceback_depth);
                    o+=(table_length-traceback_depth);
                    position=(position+table_length-traceback_depth)%table_length;
                }
            }

            i+=(table_length-traceback_depth);
        }
    }
}