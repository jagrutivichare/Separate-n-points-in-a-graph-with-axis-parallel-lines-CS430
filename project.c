#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define	MAXIMUM_POINTS	100

typedef struct coordinate point;
typedef struct line line_inst;


struct line {
	int	axis;
	int	added;
	float intercept_point;
};

struct coordinate {
	int	x_coordinate;
	int	y_coordinate;
	int	pt_con_cnt;
	int	pt_self_ix;
 int marked;
	point	**pt_connections;
};

typedef enum axis {
	X,
	Y
} axis_t;

typedef enum ret_val {
	SUCCESS,
	FILE_IS_EMPTY,
	POINTS_DONOT_MATCH_COUNT,
	FILE_NOT_FOUND,
	INS_LN_EXIST,
} ret_val_t;

static point coordinates[MAXIMUM_POINTS];
static point *sorted_x_pt[MAXIMUM_POINTS];
static point *sorted_y_pt[MAXIMUM_POINTS];
static int x_pt_taken[MAXIMUM_POINTS];
static int y_pt_taken[MAXIMUM_POINTS];

static line_inst x_line_array[MAXIMUM_POINTS];
static line_inst y_line_array[MAXIMUM_POINTS];
static line_inst *all_lines[MAXIMUM_POINTS];

static int x_line_count = 0;
static int y_line_count = 0;
static int n_lines = 0;
static int number_of_points = 0;
static int remaining_connections = 0;

static int compare_x_points(const void *ptr1, const void *ptr2)
{
	point **p1 = (point **)ptr1;
	point **p2 = (point **)ptr2;
	if ((*p1)->x_coordinate > (*p2)->x_coordinate) {
		return (1);
	}
	if ((*p1)->x_coordinate < (*p2)->x_coordinate) {
		return (-1);
	}
	return (0);
}

static int compare_y_points(const void *ptr1, const void *ptr2)
{
	point **p1 = (point **)ptr1;
	point **p2 = (point **)ptr2;
	if ((*p1)->y_coordinate > (*p2)->y_coordinate) {
		return (1);
	}
	if ((*p1)->y_coordinate < (*p2)->y_coordinate) {
		return (-1);
	}
	return (0);
}

static void sort_points()
{
	qsort(sorted_x_pt, number_of_points, sizeof (point *),
		&compare_x_points);
	qsort(sorted_y_pt, number_of_points, sizeof (point *),
		&compare_y_points);
}

/*this method establishes connection between all the points in a graph*/
static void establish_connections()
{
	int i = 0;
	while (i < number_of_points) {
		coordinates[i].pt_connections = (point**)malloc(sizeof (point *)*MAXIMUM_POINTS);
		i++;
	}
	i = 0;
	int j = 0;
	while (i < number_of_points) {
		j = 0;
		coordinates[i].pt_self_ix = i;
		while (j < number_of_points) {
			if (i != j) {
				coordinates[i].pt_connections[j] = &(coordinates[j]);
				coordinates[i].pt_con_cnt++;
				remaining_connections++;
			} else {
				coordinates[i].pt_connections[j] = NULL;
			}
			j++;
		}
		i++;
	}
}

static void release_connections()
{
	number_of_points = 0;
	n_lines = 0;
	x_line_count = 0;
	y_line_count = 0;
}

/*this method reads the input from file lying in a folder named 'input'*/
static int read_input_file(int n)
{
	char str[30];
	char *s = &str[0];
	sprintf(s, "./input/instance%.2d.txt", n);
	FILE *inst = fopen(s, "r");
 
	if (inst == NULL) {
		return (FILE_NOT_FOUND);
	}
	int ret = fscanf(inst, "%d", &number_of_points);
	int x = 0;
	int y = 0;
	int i = 0;

	if (ret == EOF) {
		return (FILE_IS_EMPTY);
	}

	while (i < MAXIMUM_POINTS) {
		x_pt_taken[i] = 0;
		y_pt_taken[i] = 0;
		i++;
	}

	i = 0;

	while (fscanf(inst, "%d %d", &x, &y) != EOF && i < MAXIMUM_POINTS) {
		coordinates[i].x_coordinate = x;
		x_pt_taken[i] = 1;
		coordinates[i].y_coordinate = y;
		y_pt_taken[i] = 1;
		coordinates[i].pt_con_cnt = 0;
		coordinates[i].marked = 0;
		i++;
	}

	if (i != number_of_points) {
		return (POINTS_DONOT_MATCH_COUNT);
	}

	int j = 0;
	while (j < number_of_points) {
		sorted_x_pt[j] = &(coordinates[j]);
		sorted_y_pt[j] = &(coordinates[j]);
		j++;
	}

	return (SUCCESS);
}

/*this method prints local optimization solution to 'output_local' folder*/
static void print_solution_to_a_file(int n, int isGreedy)
{
	char str[35];
	char *s = &str[0];
 if (isGreedy) {
   (void) sprintf(s, "./output_greedy/greedy_solution%.2d", n);
 } else {
   (void) sprintf(s, "./output_local/local_solution%.2d", n);
 }
	
	FILE *out = fopen(s, "w");
	int i = 0;
	(void) fprintf(out, "%d\n", (n_lines));
	while (i < n_lines) {
		switch (all_lines[i]->axis) {
		case X:
			(void) fprintf(out, "v ");
			break;
		case Y:
			(void) fprintf(out, "h ");
			break;
		}
		(void) fprintf(out, "%.1f\n", all_lines[i]->intercept_point);
		i++;
	}
	(void) fclose(out);
}

/*this method returns the nearest point on the left of the line*/
static int get_nearest_left_point(int axis, float inter)
{
	point **sorted;
	if (axis == X) {
		sorted = sorted_x_pt;
	} else {
		sorted = sorted_y_pt;
	}

	int i = number_of_points-1;
	float coord = 0;
	while (i >= 0) {
		if (axis == X) {
			coord = (float)sorted[i]->x_coordinate;
		} else {
			coord = (float)sorted[i]->y_coordinate;
		}

		if ((float)coord < inter) {
			return ((i+1));
		}
		i--;
	}
	return (-1);
}

/*this method returns the nearest point on the right of the line*/
static int get_nearest_right_point(int axis, float inter)
{
	point **sorted;
	if (axis == X) {
		sorted = sorted_x_pt;
	} else {
		sorted = sorted_y_pt;
	}

	int i = 0;
	float coord = 0;
	while (i < number_of_points) {
		if (axis == X) {
			coord = (float)sorted[i]->x_coordinate;
		} else {
			coord = (float)sorted[i]->y_coordinate;
		}

		if ((float)coord > inter) {
			return ((i-1));
		}
		i++;
	}
	return (-1);
}

/*this function removes the connection between two points*/
static void disconnect_points(int pt1, int pt2)
{
		if ((coordinates[pt1]).pt_connections[pt2] != NULL) {
			(coordinates[pt1]).pt_connections[pt2] = NULL;
			(coordinates[pt2]).pt_connections[pt1] = NULL;
			(coordinates[pt1]).pt_con_cnt--;
			(coordinates[pt2]).pt_con_cnt--;
			remaining_connections -= 2;
		}
}

/*this method commits the line i.e. it adds the line into the solution and disconnects all the connections which the line crosses*/
static void commit(line_inst *l)
{
	l->added = 1;
	point **sorted;
	int axis = l->axis;
	float inter = (float)l->intercept_point;
	all_lines[n_lines] = l;
	int p = get_nearest_right_point(axis, inter);
	if (axis == X) {
		sorted = sorted_x_pt;
	} else {
		sorted = sorted_y_pt;
	}
	int i = 0;
	int j = p;
	while (i <= p) {
		j = p+1;
		while (j < number_of_points) {
			disconnect_points(sorted[i]->pt_self_ix,
				sorted[j]->pt_self_ix);
			j++;
		}
		i++;
	}
	n_lines++;
}

/*this method checks for the connection(edge) between two points*/
static int check_connection(line_inst *ln)
{
	int axis = ln->axis;
	float inter = ln->intercept_point;
	int p = get_nearest_right_point(axis, inter);
	point **sorted;

	if (axis == X) {
		sorted = &(sorted_x_pt[0]);
	} else {
		sorted = &(sorted_y_pt[0]);
	}

	int i = 0;
	int j = p+1;
	while (i < (p+1)) {
		j = p+1;
		while (j < number_of_points) {
			if ((coordinates[(sorted[i]->pt_self_ix)].
				pt_connections[(sorted[j]->pt_self_ix)]) != NULL) {
				return (1);
			}
			j++;
		}
	i++;
	}
	return (0);
}

/*this function marks all the coordinate points back to zero*/
static void mark_all_zero() {
  for (int i = 0 ; i < number_of_points; i++) {
    coordinates[i].marked = 0;
  }
} 

/*this function check the feasibility of a solution by checking if any of the coordinate points are not marked. If there is any 
such point then the solution is not feasible */
static int check_if_feasible() {
  for (int i = 0 ; i < number_of_points; i++) {
    if (!coordinates[i].marked) {
      return 0;
    }
  }
  return 1;
} 

/*this function marks the coordinate points which are divided by the line passed in the parameter of this function*/ 
static void coord_mark_one(line_inst *ln) {
  int axis = ln->axis;
  float inter = ln->intercept_point;
  int p = get_nearest_right_point(axis, inter);
  int q = get_nearest_left_point(axis, inter);
  coordinates[p+1].marked = 1;
  coordinates[q-1].marked = 1;
}

/*this function checks if the removal of first two lines followed by addition of third line results into another feasible solution. 
It doesnot actually remove the lines. It just doesnot consider the first two lines while calculating feasibilty. Feasibilty 
is calculated my marking coordinate points. If all the coordinate points are marked while ignoring first two lines, then the soltion is
said to be feasible*/
static int check_feasibility(line_inst *third, line_inst *first, line_inst *second) {
  int third_axis = third->axis;
  int first_axis = first->axis;
  int second_axis = second->axis;
  
  if (third_axis == X && first_axis == X && second_axis == X) {
    for (int j = 0; j < x_line_count; j++) {
      if (&(x_line_array[j]) != third && &(x_line_array[j]) != first && &(x_line_array[j]) != second) {
        coord_mark_one(&(x_line_array[j]));
      }
    }
    for (int j = 0; j < y_line_count; j++) {
      coord_mark_one(&(y_line_array[j]));
    }
  } else  if (third_axis == Y && first_axis == X && second_axis == X) {
    for (int j = 0; j < x_line_count; j++) {
      if (&(x_line_array[j]) != first && &(x_line_array[j]) != second) {
        coord_mark_one(&(x_line_array[j]));
      }
    }
    for (int j = 0; j < y_line_count; j++) {
      if (&(y_line_array[j]) != third) {
        coord_mark_one(&(y_line_array[j]));
      }
    }
  } else if (third_axis == X && first_axis == Y && second_axis == Y) {
    for (int j = 0; j < y_line_count; j++) {
      if (&(y_line_array[j]) != first && &(y_line_array[j]) != second) {
        coord_mark_one(&(y_line_array[j]));
      }
    }
    for (int j = 0; j < x_line_count; j++) {
      if (&(x_line_array[j]) != third) {
        coord_mark_one(&(x_line_array[j]));
      }
    }
  } else if (third_axis == Y && first_axis == Y && second_axis == Y) {
    for (int j = 0; j < y_line_count; j++) {
      if (&(y_line_array[j]) != first && &(y_line_array[j]) != first && &(y_line_array[j]) != second) {
        coord_mark_one(&(y_line_array[j]));
      }
    }
    for (int j = 0; j < x_line_count; j++) {
      coord_mark_one(&(x_line_array[j]));
    }
  } else  if (third_axis == X && first_axis == X && second_axis == Y) {
    for (int j = 0; j < x_line_count; j++) {
      if (&(x_line_array[j]) != third && &(x_line_array[j]) != first) {
        coord_mark_one(&(x_line_array[j]));
      }
    }
    for (int j = 0; j < y_line_count; j++) {
      if (&(y_line_array[j]) != second) {
        coord_mark_one(&(y_line_array[j]));
      }
    }
  } else  if (third_axis == Y && first_axis == X && second_axis == Y) {
    for (int j = 0; j < x_line_count; j++) {
      if (&(x_line_array[j]) != first) {
        coord_mark_one(&(x_line_array[j]));
      }
    }
    for (int j = 0; j < y_line_count; j++) {
      if (&(y_line_array[j]) != third && &(y_line_array[j]) != second) {
        coord_mark_one(&(y_line_array[j]));
      }
    }
  }
  int feasible = check_if_feasible();
  if (!feasible) {
    float inter = third->intercept_point;
    int p = get_nearest_right_point(third_axis, inter);
    int q = get_nearest_left_point(third_axis, inter);
    coordinates[p+1].marked = 1;
    coordinates[q-1].marked = 1;
    if (check_if_feasible()) {
      mark_all_zero();
      return 1;
    } else {
      mark_all_zero();
      return 0;
    }
  } else {
    mark_all_zero();
    return (0);
  }
} 

/*this function removes lines from the graph*/
static void remove_lines(line_inst *ln, int position) {
  int axis = ln->axis;
  if(axis == X) {
    for (int c = position ; c < x_line_count - 1 ; c++){
      x_line_array[c] = x_line_array[c+1]; 
    }
    x_line_count--;
  } else {
    for (int c = position ; c < y_line_count - 1 ; c++){
      y_line_array[c] = y_line_array[c+1]; 
    }
    y_line_count--;
  }
}

static int check_if_already_exist(line_inst *ln) {
  for (int i = 0; i < n_lines; i++) {
    if (all_lines[i]->axis == ln->axis && all_lines[i]->intercept_point == ln->intercept_point) {
      return 1;
    }
  }
  return 0;
}

/*this function compares two lines with another line checks whether two lines can be replaced with the other line by checking if ]
the solution it is fiving is feasible*/
static void find_best_solution() { 
  //checking all combinations of two vertical lines with third line
  for (int first = 0; first < x_line_count-1; first++) {
    for (int second = first+1; second < x_line_count; second++) {
      int third_commited = 0;
      int third_val;
      for (int third = 0; third < x_line_count; third++) {
        int feasible = check_feasibility(&(x_line_array[third]), &(x_line_array[first]), &(x_line_array[second]));
        if (feasible) {
          if (!check_if_already_exist(&(x_line_array[third]))) {
            commit(&(x_line_array[third]));      
          } 
          third_val = third;
          third_commited = 1;
          third = x_line_count;
        }       
      }
      if (!third_commited) {
        for (int third = 0; third < y_line_count; third++) {
          int feasible = check_feasibility(&(y_line_array[third]), &(x_line_array[first]), &(x_line_array[second]));
          if (feasible) {
            if (!check_if_already_exist(&(y_line_array[third]))) {
              commit(&(y_line_array[third]));
            }     
            third_commited = 1;
            third = y_line_count;
          }    
        }
      }
      if (third_commited) {
        int deletedFirst = 0;
        if (first != third_val) {
          remove_lines(&(x_line_array[first]), first);
          deletedFirst = 1;
        }
        if (second != third_val) {
          if (deletedFirst) {
            remove_lines(&(x_line_array[second-1]), second-1);
          } else {
            remove_lines(&(x_line_array[second]), second);
          } 
        } 
      } else {
        if (!check_if_already_exist(&(x_line_array[first]))) {
          commit(&(x_line_array[first]));
        }
        if (!check_if_already_exist(&(x_line_array[second]))) {
          commit(&(x_line_array[second]));
        }
      }
    }
  }
  //checking all combinations of two horizontal lines with third line
  for (int first = 0; first < y_line_count-1; first++) {
    for (int second = first+1; second < y_line_count; second++) { 
      int third_commited = 0;
      for (int third = 0; third < x_line_count; third++) {
        int feasible = check_feasibility(&(x_line_array[third]), &(y_line_array[first]), &(y_line_array[second]));
        if (feasible) {
          if (!check_if_already_exist(&(x_line_array[third]))) {
            commit(&(x_line_array[third]));
          }
          third_commited = 1;
          third = x_line_count;
        }       
      }
      int third_val;
      if (!third_commited) {
        for (int third = 0; third < y_line_count; third++) {
          int feasible = check_feasibility(&(y_line_array[third]), &(y_line_array[first]), &(y_line_array[second]));
          if (feasible) {
            if (!check_if_already_exist(&(y_line_array[third]))) {
              commit(&(y_line_array[third]));
            }
            third_commited = 1;
            third_val = third;
            third = y_line_count;
          }    
        }
      }
      if (third_commited) {
        int deletedFirst = 0;
        if (first != third_val) {
          remove_lines(&(y_line_array[first]), first);
          deletedFirst = 1;
        }
        if (second != third_val) {
          if (deletedFirst) {
            remove_lines(&(y_line_array[second-1]), second-1);
          } else {
            remove_lines(&(y_line_array[second]), second);
          }
        }       
      } else {
        if (!check_if_already_exist(&(y_line_array[first]))) {
          commit(&(y_line_array[first]));
        }
        if (!check_if_already_exist(&(y_line_array[second]))) {
          commit(&(y_line_array[second]));
        }       
      }
    }
  }
  //checking all combinations of a vertical and horizontal line with third line
  for (int first = 0; first < x_line_count; first++) {
    for (int second = 0; second < y_line_count; second++) { 
      int third_commited = 0;
      int third_x;
      for (int third = 0; third < x_line_count; third++) {
        int feasible = check_feasibility(&(x_line_array[third]), &(x_line_array[first]), &(y_line_array[second]));
        if (feasible) {
          if (!check_if_already_exist(&(x_line_array[third]))) {
            commit(&(x_line_array[third]));
          }
          third_commited = 1;
          third_x = third;
          third = x_line_count;
        }       
      }
      int third_y;
      if (!third_commited) {
        for (int third = 0; third < y_line_count; third++) {
          int feasible = check_feasibility(&(y_line_array[third]), &(x_line_array[first]), &(y_line_array[second]));
          if (feasible) {
            if (!check_if_already_exist(&(y_line_array[third]))) {
              commit(&(y_line_array[third]));
            }
            third_commited = 1;
            third_y = third;
            third = y_line_count;
          }    
        }
      }
      if (third_commited) {
        if (first != third_x) {
          remove_lines(&(x_line_array[first]), first);
        }
        if (second != third_y) {
          remove_lines(&(y_line_array[second]), second);
        }
      } else {
        if (!check_if_already_exist(&(x_line_array[first]))) {
          commit(&(x_line_array[first]));
        }
        if (!check_if_already_exist(&(y_line_array[second]))) {
          commit(&(y_line_array[second]));
        }
      }
    }
  }
}

/*this function divides X and Y axis between every two points on the graph for the given range at a specific interval*/
static void divide_entire_axis(int axis, int from, int to)
{
  if (axis == X) { 
    for (int i = from; i < to; i++) {
     line_inst *cur_ln;
     cur_ln = &(x_line_array[i]);
     cur_ln->axis = X;
     cur_ln->intercept_point = i+1.5;
     cur_ln->added = 0;
     x_line_count++;
    }
  } else {
    for (int i = from; i < to; i++) {
     line_inst *cur_ln;
     cur_ln = &(y_line_array[i]);
     cur_ln->axis = Y;
     cur_ln->intercept_point = i+1.5;
     cur_ln->added = 0;
     y_line_count++;
    }
  }
}

/*this function divides X and Y axis into half recursively. Since when a line is drawn exacly at the mid of the graph it 
disconnects most of the points*/
static void divide_axis(int axis, int from, int to)
{
	if (from == to) {
		return;
	}
	int range;
	int mid;
	float point_1;
	float point_2;
	point *pt1;
	point *pt2;
	float dist;
	float mid_dist;
	float mid_coordinate;
	line_inst *axis_parallel_line;
	point **sorted;
	if (axis == X) {
		sorted = &(sorted_x_pt[0]);
		range = to - from;
		if (range == 0 || range == 1) {
			return;
		}

		if (range%2) {
			mid = ((range-1)/2);
		} else {
			mid = range/2;
		}

		if (mid) {
			pt1 = (sorted[(from + (mid))]);
			pt2 = (sorted[(from + (mid+1))]);
			point_1 = (float)pt1->x_coordinate;
			point_2 = (float)pt2->x_coordinate;
			dist = point_2 - point_1;
			mid_dist = dist/2;
			mid_coordinate = point_1 + mid_dist;
			axis_parallel_line = &(x_line_array[x_line_count]);
			axis_parallel_line->axis = X;
			axis_parallel_line->intercept_point = mid_coordinate;
			axis_parallel_line->added = 0;
			x_line_count++;
			if (range != 2) {
				divide_axis(axis, from, (from+mid));
				divide_axis(axis, (from+mid), to);
			}
		}
		return;
	} else {
		sorted = &(sorted_y_pt[0]);
		range = to - from;
		if (range == 0 || range == 1) {
			return;
		}
		if (range%2) {
			mid = ((range-1)/2);
		} else {
			mid = range/2;
		}

		if (mid) {
			pt1 = sorted[(from + (mid))];
			pt2 = sorted[(from + (mid+1))];
			point_1 = (float)pt1->y_coordinate;
			point_2 = (float)pt2->y_coordinate;
			dist = point_2 - point_1;
			mid_dist = dist/2;
			mid_coordinate = point_1 + mid_dist;
			axis_parallel_line = &(y_line_array[y_line_count]);
			axis_parallel_line->axis = Y;
			axis_parallel_line->intercept_point = mid_coordinate;
			axis_parallel_line->added = 0;
			y_line_count++;
			if (range != 2) {
				divide_axis(axis, from, (from+mid));
				divide_axis(axis, (from+mid), to);
			}
		}
		return;
	}
}

/*run local optimization program*/
static void run_local(int instance_count) 
{
  int file_status = read_input_file(instance_count);
		switch (file_status) {
		case FILE_NOT_FOUND:
			if(instance_count > 1){
				(void) fprintf(stdout, "The %d instance/s of the input are read!", instance_count-1);
			}
			else{
				(void) fprintf(stderr, "Error! instance[%d] file not found", instance_count);
			}
			exit(0);
			break;
		case POINTS_DONOT_MATCH_COUNT:
			(void) fprintf(stderr, "Error! Points in instance[%d] file does not match the total count mentioned at the start",
				instance_count);
			exit(0);
			break;
		case FILE_IS_EMPTY:
			(void) fprintf(stderr, "Error! instance[%d] file is empty",
				instance_count);
			exit(0);
			break;
		}

		sort_points();
		establish_connections();
  		divide_entire_axis(X, 0, (number_of_points-1));
		divide_entire_axis(Y, 0, (number_of_points-1));
 		find_best_solution();
  
		print_solution_to_a_file(instance_count, 0);
		(void) printf("Output for instance%.2d file has been generated in output_local folder\n", instance_count);
		release_connections();
}

/*run greedy algorithm*/
static void run_greedy(int instance_count)
{
		int file_status = read_input_file(instance_count);
		switch (file_status) {
		case FILE_NOT_FOUND:
			if(instance_count > 1){
				(void) fprintf(stdout, "The %d instance/s of the input are read!", instance_count-1);
			}
			else{
				(void) fprintf(stderr, "Error! instance[%d] file not found", instance_count);
			}
			exit(0);
			break;
		case POINTS_DONOT_MATCH_COUNT:
			(void) fprintf(stderr, "Error! Points in instance[%d] file does not match the total count mentioned at the start",
				instance_count);
			exit(0);
			break;
		case FILE_IS_EMPTY:
			(void) fprintf(stderr, "Error! instance[%d] file is empty",
				instance_count);
			exit(0);
			break;
		}

		sort_points();
		establish_connections();

		divide_axis(X, 0, (number_of_points-1));
		divide_axis(Y, 0, (number_of_points-1));

		int count_x = 0;
		int count_y = 0;
		int connection = 0;
		while (count_x < x_line_count && count_y < y_line_count) {
			connection = check_connection(&(x_line_array[count_x]));
			if (connection) {
				commit(&(x_line_array[count_x]));
			}
			count_x++;

			connection = check_connection(&(y_line_array[count_y]));
			if (connection) {
				commit(&(y_line_array[count_y]));
			}
			count_y++;
		}

		print_solution_to_a_file(instance_count, 1);
		(void) printf("Output for instance%.2d file has been generated in output_greedy folder\n", instance_count);
		release_connections();
}

int main()
{
	int instance_count = 1;
	while (instance_count < 100) {
  		run_local(instance_count);
  		run_greedy(instance_count);
  		instance_count++;
	}
}
