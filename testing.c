
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include "multiThreadSorter_thread.h"
#include <errno.h>
#include <limits.h>
#include <pthread.h>

int main(int argc, char *argv[] ){

	char headers[28][40] = {"color","director_name","num_critic_for_reviews",
"duration","director_facebook_likes","actor_3_facebook_likes","actor_2_name",
"actor_1_facebook_likes","gross","genres","actor_1_name","movie_title",
"num_voted_users","cast_total_facebook_likes","actor_3_name",
"facenumber_in_poster","plot_keywords","movie_imdb_link","num_user_for_reviews",
"language","country","content_rating","budget","title_year","actor_2_facebook_likes",
"imdb_score","aspect_ratio","movie_facebook_likes"};

	printf("%s\n",headers[3]);
}