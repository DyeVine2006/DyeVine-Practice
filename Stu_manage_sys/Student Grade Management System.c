#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct student {
	char name[20];
	char gender[3];
	float grd_EN;
	float grd_math;
	float grd_C;
	float sum;
	float average;
	struct student* next;
}STU;


STU* READ(FILE*);
void INPUT(STU*);
void INQUIRE(STU*);
void MODIFY(STU*);
void DISPLAY(STU*);
void WRITE(STU*,FILE*);

int main(void)
{
	//This program stores and outputs alphabetically by names.
	FILE* fp;
	STU* h;
	char ch;
	fp = fopen("D:\\Grade.txt", "r");
	if (fp == NULL) {
		printf("Fail to open the file!");
		exit(0);
	}
	h = READ(fp);
	fclose(fp);

	printf("Welcome to use Student Grade Management System.\n\n");
	printf("Enter 1 to 4 or \"q\" to carry out the corresponding operations:\n");
	printf("1: Input Students' Information\n\
2: Inquire Students' Information\n\
3: Modify Students' Information\n\
4: Display All the Students' Information\n\
q: Quit\n");

	do {
		switch (ch = getchar())
		{
		case'1':INPUT(h);break;
		case'2':INQUIRE(h);break;
		case'3':MODIFY(h);break;
		case'4':DISPLAY(h);break;
		case'q':break;
		default:printf("Please enter the specified characters.\n");break;
		}

		fp = fopen("D:\\Grade.txt", "w");
		if (fp == NULL) {
			printf("Fail to open the file!");
			exit(0);
		}
		WRITE(h, fp);
		fclose(fp);
	} while (ch != 'q');
	
	printf("\nLooking forward to your next vist!");
	return 0;
}

STU* READ(FILE* fp)
{
	STU* head, * p, * q;
	head = (STU*)malloc(sizeof(STU));
	head->name[0] = '\0';
	p = head;
	q = (STU*)malloc(sizeof(STU));
	p->next = q;
	while (fscanf(fp, "%[^,],%[^,],%f,%f,%f,%f,%f\n", q->name, q->gender, &q->grd_EN, &q->grd_math, &q->grd_C, &q->sum, &q->average) == 7) {
		p = q;
		q = (STU*)malloc(sizeof(STU));
		p->next = q;
	}
	p->next = NULL;
	free(q);
	q = NULL;

	return head;
}

void INPUT(STU* h)
{
	STU* p, * q, * r;
	r = (STU*)malloc(sizeof(STU));

	while (getchar() != '\n');
	printf("Name: ");
	fgets(r->name, 20, stdin);
	r->name[strcspn(r->name, "\n")] = '\0';
	printf("Gender(M/F): ");
	scanf("%1s", r->gender);
	while (getchar() != '\n');
	printf("Grade of English: ");
	scanf("%f", &r->grd_EN);
	while (getchar() != '\n');
	printf("Grade of Math: ");
	scanf("%f", &r->grd_math);
	while (getchar() != '\n');
	printf("Grade of C Language: ");
	scanf("%f", &r->grd_C);
	while (getchar() != '\n');
	r->sum = r->grd_EN + r->grd_math + r->grd_C;
	r->average = r->sum / 3.0;

	q = h;
	p = h->next;
	while (1) {
		if (p == NULL) {
			q->next = r;
			r->next = NULL;
			break;
		}
		else if (strcmp(q->name,r->name)<=0 && strcmp(p->name, r->name)>=0){
			q->next = r;
			r->next = p;
			break;
		}

		q = p;
		p = q->next;
	}

	printf("\nDone! Continue or quit?\n");
	printf("Enter 1 to 4 or \"q\" to carry out the corresponding operations:\n");
}

void INQUIRE(STU* h)
{
	char Name[20];
	int n;
	STU* p;
	printf("Name of Whom you'd like to inquire: ");
	while (getchar() != '\n');
	fgets(Name, 20, stdin);
	Name[strcspn(Name, "\n")] = '\0';

	p = h->next;
	while (p != NULL) {
		if (strcmp(Name, p->name) == 0) {
			if (strlen(p->name) < 8) {
				printf("Name\tGender\tEnglish\tMath\tC\tSum\tAverage\n");
				printf("%s\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", p->name, p->gender, p->grd_EN, p->grd_math, p->grd_C, p->sum, p->average);

			}
			else {
				printf("Name\t\tGender\tEnglish\tMath\tC\tSum\tAverage\n");
				printf("%s\t%s\t%f\t%f\t%f\t%f\t%f\n", p->name, p->gender, p->grd_EN, p->grd_math, p->grd_C, p->sum, p->average);
			}
			break;
		}
		p = p->next;
	}
	if (p == NULL) {
		printf("Not Found\n");
	}

	printf("\nDone! Continue or quit?\n");
	printf("Enter 1 to 4 or \"q\" to carry out the corresponding operations:\n");
}

void MODIFY(STU* h)
{
	char ch, Name[20];
	int n;
	STU* p, * q, * r;
	printf("Name of Whom you'd like to inquire: ");
	while (getchar() != '\n');
	fgets(Name, 20, stdin);
	Name[strcspn(Name, "\n")] = '\0';

	q = h;
	p = h->next;
	while (p != NULL) {
		if (strcmp(Name, p->name) == 0) {
			q->next = p->next;
			p->next = NULL;
			r = p;
			p = h->next;
			q = h;

			printf("Enter 1 to 5, \"d\" or \"q\" to choose the option:\n");
			printf("1: Change the Name\n\
2: Change the Gender\n\
3: Change the Grade of English\n\
4: Change the Grade of Math\n\
5: Change the Grade of C Language\n\
d: Delete\n\
q: Quit\n");

			do {
				switch (ch = getchar())
				{
				case'1':
					printf("New Name: ");
					while (getchar() != '\n');
					fgets(r->name, 20, stdin);
					r->name[strcspn(r->name, "\n")] = '\0';
					printf("Done! Change someting else or quit?\n");
					printf("Enter 1 to 5, \"d\"  or \"q\" to choose the option:\n");
					break;
				case'2':
					printf("New Gender: ");
					scanf("%s", r->gender);
					printf("Done! Change someting else or quit?\n");
					printf("Enter 1 to 5, \"d\"  or \"q\" to choose the option:\n");
					while (getchar() != '\n');
					break;
				case'3':
					printf("New Score: ");
					scanf("%f", &r->grd_EN);
					while (getchar() != '\n');
					printf("Done! Change someting else or quit?\n");
					printf("Enter 1 to 5, \"d\"  or \"q\" to choose the option:\n");
					break;
				case'4':
					printf("New Score: ");
					scanf("%f", &r->grd_math);
					while (getchar() != '\n');
					printf("Done! Change someting else or quit?\n");
					printf("Enter 1 to 5, \"d\"  or \"q\" to choose the option:\n");
					break;
				case'5':
					printf("New Score: ");
					scanf("%f", &r->grd_C);
					while (getchar() != '\n');
					printf("Done! Change someting else or quit?\n");
					printf("Enter 1 to 5, \"d\"  or \"q\" to choose the option:\n");
					break;
				case'd':
					while (getchar() != '\n');
					free(r);
					r = NULL;
					printf("Delete Successfully!");
					ch = 'q';
					break;
				case'q':
					while (getchar() != '\n');
					break;
				default:printf("Please enter the specified characters.");break;
				}

				if (ch >= '3' && ch <= '5') {
					r->sum = r->grd_EN + r->grd_math + r->grd_C;
					r->average = r->sum / 3.0;
				}

				while (ch >= '1' && ch <= '5') {
					if (p == NULL) {
						q->next = r;
						r->next = NULL;
						break;
					}
					else if (strcmp(q->name, r->name) <= 0 && strcmp(p->name, r->name) >= 0) {
						q->next = r;
						r->next = p;
						break;
					}

					q = p;
					p = q->next;
				}
			} while (ch != 'q');

			break;
		}
		q = p;
		p = q->next;
	}
	if (p == NULL) {
		printf("Not Found\n");
	}

	printf("\nDone! Continue or quit?\n");
	printf("Enter 1 to 4 or \"q\" to carry out the corresponding operations:\n");
}

void DISPLAY(STU* h)
{
	STU* p = h->next;
	printf("Name\t\tGender\tEnglish\tMath\tC\tSum\tAverage\n");
	while (p != NULL) {
		if(strlen(p->name)<8)
			printf("%s\t\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", p->name, p->gender, p->grd_EN, p->grd_math, p->grd_C, p->sum, p->average);
		else
			printf("%s\t%s\t%.2f\t%.2f\t%.2f\t%.2f\t%.2f\n", p->name, p->gender, p->grd_EN, p->grd_math, p->grd_C, p->sum, p->average);
		p = p->next;
	}

	printf("\nDone! Continue or quit?\n");
	printf("Enter 1 to 4 or \"q\" to carry out the corresponding operations:\n");
}

void WRITE(STU* h, FILE* fp)
{
	STU* p = h->next;
	while (p != NULL) {
		fprintf(fp, "%s,%s,%f,%f,%f,%f,%f\n", p->name, p->gender, p->grd_EN, p->grd_math, p->grd_C, p->sum, p->average);
		p = p->next;
	}
}