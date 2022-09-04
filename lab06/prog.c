#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define KOEF 20000

struct string_list {
	char **data;
	size_t length;
	size_t capacity;
};

int init_list(struct string_list *list) {
	list->capacity = 6;
	list->length = 0;
	list->data = (char**) malloc(sizeof(char*) * list->capacity);
	if(list->data == NULL)
		return 1;
	return 0;
}

void destroy_list(struct string_list *list) {
	for(size_t index = 0; index < list->length; index++)
		free(list->data[index]);
	free(list->data);
	list->data = NULL;
	list->length = list->capacity = 0;
}

int add_string(struct string_list *list, char *str) {
	if(list->capacity == list->length) {
		list->capacity *= 2;
		char **new_data = (char**) realloc(list->data,
				sizeof(char*) * list->capacity);
		if(new_data == NULL) {
			list->capacity /= 2;
			return 1;
		}
		list->data = new_data;
	}
	list->data[list->length] = str;
	(list->length)++;
	return 0;
}

void *sleepsort(void *args) {
	char *str = (char*) args;
	int length = strlen(str);
	usleep(length * KOEF);
	printf("%s", str);
	pthread_exit(NULL);
}

int main() {
	pthread_attr_t attrs;
	if(pthread_attr_init(&attrs) != 0) {
		perror("pthread attr init");
		exit(-1);
	}

	struct string_list list;
	if(init_list(&list) != 0) {
		perror("list init");
		pthread_attr_destroy(&attrs);
		exit(-1);
	}
	while(1) {
		size_t len = 0;
		char *str = NULL;
		getline(&str, &len, stdin);
		if(str[0] == '\0') {
			free(str);
			break;
		}
		add_string(&list, str);
	}
	pthread_t *threads = (pthread_t*) malloc(sizeof(pthread_t) * list.length);
	if(threads == NULL) {
		perror("thread malloc");
		destroy_list(&list);
		pthread_attr_destroy(&attrs);
		exit(-1);
	}

	for(size_t index = 0; index < list.length; index++)
		pthread_create(threads + index, &attrs, sleepsort, list.data[index]);

	for(size_t index = 0; index < list.length; index++)
		pthread_join(threads[index], NULL);
	
	pthread_attr_destroy(&attrs);
	destroy_list(&list);
	free(threads);
	pthread_exit(NULL);
}
