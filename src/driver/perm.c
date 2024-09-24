/* Sequential schedule generation for driver.cvl
 * Created Dec-5-2017, last modified Jul-24-2021
 * Wenhao Wu, Josh Davis, Stephen Siegel
 * Verified Software Lab, CIS Dept.,
 * University of Delaware
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "schedule.h"
#include "perm.h"

void perm_print(int idx_perms, int size_perm, int** perms) {
  printf("Perm[%d]:\t", idx_perms);
  for (int j = 0; j < size_perm; j++){
    printf("%d, ", perms[idx_perms][j]);
  }
  printf("\n");
}

void perm_print_all(int num_perms, int size_perm, int** perms) {
  for (int i = 0; i < num_perms; i++){
    printf("Perm[%d]:\t", i);
    for (int j = 0; j < size_perm; j++){
      printf("%d ", perms[i][j]);
    }
    printf("\n");
  }
}

/*  Factorial(n) */
static int fact(int n){
  int result = 1;
	
  for (int i = 1; i <= n; i++){
    result *= i;
  }
  return result;
}

/* Computes the number of permutations generated by the counts array */
int perm_calc_num(int n, int * counts){
  int sum = 0;
  for (int i = 0; i < n; i++){
    sum += counts[i];
  }
  int numerator = fact(sum);
  int denom = 1;
  for (int i = 0; i < n; i++){
    denom *= fact(counts[i]);
  }
  return numerator/denom;
}

// ignore
static int* create_perm_array(int n, int* counts, int* perm_array,
                              int a, int indx, int count){
	
  for (int i = a; i < n; i++){
    if(counts[i] > 1 && count == 0){
      perm_array[i] += i;
      indx++;
      count++;
      counts[i] -= 1;
      create_perm_array(n, counts, perm_array, a, indx, count);
    }
    else if(counts[i] > 1 && count != 0){
      perm_array[count] += i;
      indx++;
      count++;
      counts[i] -= 1;
      create_perm_array(n, counts, perm_array, a, indx, count);
    }
    else if(counts[i] <= 1 && count > 0){
      perm_array[count] = i;
      indx++;
      counts[i] += count;
      count=0;
      create_perm_array(n, counts, perm_array, (a+1), indx, count);
    }
    else {
      perm_array[indx] += i;
      indx++;
    }
  }
  return perm_array;
}

static void perm_aux(int **table,
                     int i0,
                     int j0,
                     int len,
                     int * counts) {
  for (int i=0; i<len; i++) {
    if (counts[i] != 0) {
      int new_counts[len];
      for (int j=0; j<len; j++) {
	new_counts[j] = j==i ? counts[j]-1 : counts[j];
      }
      int np = perm_calc_num(len, new_counts);
      for (int j=0; j<np; j++) {
	table[i0+j][j0] = i;
      }
      perm_aux(table, i0, j0+1, len, new_counts);
      i0 += np;
    }
  }
}

/* Computes the permutations generated by counts array */
int** perm_compute(int n, int * counts){
  int num_rows = perm_calc_num(n, counts);
  int sum = 0;
  for (int i = 0; i < n; i++){
    sum += counts[i];
  }
  int num_cols = sum;
  int** result = malloc(num_rows * sizeof(int*));
  for (int i = 0; i < num_rows; i++){
    result[i] = malloc(num_cols * sizeof(int));
  }
  perm_aux(result, 0, 0, n, counts);
  return result;
}

/* Similar to perm_compute, but for a "stuck" schedule.  Thread tid is
   stuck, i.e., it has called, but not returned from a step in its
   schedules.  Other threads will either be stuck or terminated. */
int** perm_compute_stuck(int n, int * counts, int tid) {
  // TODO: LEFT OFF HERE!!!!!

  // IS THIS NEEDED?


  
}

/* Linearizability check, returns false if the next op to be added has a stop time
   that is earlier than the start time of some other op that has already been added
   to the permuation. */
static bool check_times(int* row,
                        int curr_total_op_count,
                        int tid_next,
                        schedule_t schedule,
                        int nthread) {
  int tid_next_op_index = 0;
  int ops_index[nthread];
  step_t** steps = schedule.steps;
  for (int i = 0; i < nthread; i++) ops_index[i] = 0;
  for (int i = 0; i < curr_total_op_count - 1; i++) {
    if (row[i] == tid_next) tid_next_op_index++;
  } 
  int next_stop_time = steps[tid_next][tid_next_op_index].stop_time;
  if (next_stop_time < 0) {
    // if next_stop_time == -2, treat it as +infty, since the thread
    // never returned from the call.  There is no way it could violate
    // the linearizability condition, since all the start times are
    // finite numbers.
    assert(next_stop_time == -2);
    return true;
  }
  for (int i = 0; i < curr_total_op_count - 1; i++) {
    if (next_stop_time < steps[row[i]][ops_index[row[i]]++].start_time) {
      return false;
    }
  }
  return true;
}

/* Analogous to perm_aux but includes linearizability check */
static void linearizable_aux(int **table,
                             int i0,
                             int j0,
                             int nthread,
                             int * counts,
                             schedule_t schedule) {
  for (int i=0; i<nthread; i++) {
    if (counts[i] != 0) {
      int new_counts[nthread];
      for (int j=0; j<nthread; j++) {
	new_counts[j] = j==i ? counts[j]-1 : counts[j];
      }
      int np = perm_calc_num(nthread, new_counts);
      for (int j=0; j<np; j++) {
	if (table[i0+j][0] >= 0) {
	  if (check_times(table[i0+j], j0, i, schedule, nthread)) {
	    table[i0+j][j0] = i;
	  } else {
	    table[i0+j][0] = -1;
	  }
	}
      }
      linearizable_aux(table, i0, j0+1, nthread, new_counts, schedule);
      i0 += np;
    }
  }
}

/* Computes the permutations generated by counts array, filtering out
   permuations that fail to meet the linearizability condition. Filtered
   perms are flagged with a -1 in their first index. */
int** perm_compute_linear(schedule_t schedule, int * counts) {
  int n = schedule.nthread;
  //int* counts = schedule.nsteps;
  step_t** steps = schedule.steps;
  int num_rows = perm_calc_num(n, counts);
  int sum = schedule.nstep;
  int num_cols = sum;
  int** result = (int **) malloc(num_rows * sizeof(int*));
  for (int i = 0; i < num_rows; i++) {
    result[i] = (int *) malloc(num_cols * sizeof(int));
    for (int j = 0; j < num_cols; j++) {
      result[i][j] = 0;
    }
  }
  linearizable_aux(result, 0, 0, n, counts, schedule);
  /*
  // Consider copying the old array into a new smaller array without
  // the trimmed perms?
  int** old_result = result;
  int new_num_rows = 0;
  for (int i = 0; i < num_rows; i++) {
  if (result[i][0] >= 0) new_num_rows++;
  }
  */
  return result;
}


//  TODO: REMOVE the last stuck step from the result and
// then compute all permutations and check for linearizability
// and check last step makes oracle stuck.

/* Like perm_compute_linear, except for a stuck schedule.  Let H[tid]
   be the history obtained by removing all -2 steps (i.e., all steps
   which were never executed or did not complete), except thread tid
   keeps its first -2 step.  Generate all (linearizable) perms of that
   schedule. 

   Note: this function should only be called for a thread tid which
   has *not* terminated (i.e., thread tid should be "stuck").  See
   Burckhardt, Dern, Musuvathi, Tan, 2010 "Line-up: a complete and
   automatic linearizability checker".
*/
int ** perm_compute_linear_stuck(schedule_t schedule, int tid) {
  int n = schedule.nthread;
  int counts[n];
  step_t** steps = schedule.steps;
  for (int i=0; i<n; i++) {
    const ns = schedule.nsteps[i];
    int j=0;
    for (; j<ns; j++) {
      if (steps[i][j].result == -2)
        break;
    }
    // either steps[i][j+1]==-2 or j==ns
    if (i == tid) {
      assert(j<ns);
      counts[i] = j+1;
    } else
      counts[i] = j;
  }
  
  int num_rows = perm_calc_num(n, counts);
  int num_cols = 0;
  for (int i=0; i<n; i++)
    num_cols += counts[i];
  int** result = (int **) malloc(num_rows * sizeof(int*));
  for (int i = 0; i < num_rows; i++) {
    result[i] = (int *) malloc(num_cols * sizeof(int));
    for (int j = 0; j < num_cols; j++) {
      result[i][j] = 0;
    }
  }
  linearizable_aux(result, 0, 0, n, counts, schedule);
}




static void test_1(int n, int* counts){
  int sum = 0;
  int a = 0;
  int indx = 0;
  int count = 0;
	
  for (int i = 0; i < n; i++){
    sum += counts[i];
  }
	
  int* perm_array = malloc(sum * sizeof(int));
	
  perm_array = create_perm_array(n, counts, perm_array, a, indx, count);
  printf("create_perm_array(3, (2,1,1), perm_array, 0, 0, 0) = %d\n", *perm_array);
}

#ifdef _PERMS_MAIN
int main() {
  int test_n = 4;
  int* test_counts;
  test_counts = (int*) malloc(sizeof(int)*test_n);
  test_counts[0] = 1;
  test_counts[1] = 1;
  test_counts[2] = 2;
  test_counts[3] = 1;
  //test_1(test_n, test_counts);
  int** result = perm_compute(test_n, test_counts);
  int num_rows = perm_calc_num(test_n, test_counts);
  int num_cols = 0;
  for (int i = 0; i < test_n; i++) {
    num_cols += test_counts[i];
  }
  for (int i = 0; i < num_rows; i++) {
    for (int j = 0; j < num_cols; j++) {
      printf("%d ", result[i][j]);
    }
    printf("\n");
  }
}
#endif
