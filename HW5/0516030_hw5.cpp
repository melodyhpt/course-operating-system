#include <fstream>
#include <iostream>
#include <iomanip>
#include <sys/time.h>

using namespace std;

struct node {
  unsigned int num;
  node *prev = NULL;
  node *next = NULL;
};

struct page {
  unsigned int num;
  page *next = NULL;
  node *position = NULL;
};

page **find_page = new page *[100];

int main() {
  char filename[30];
  cout << "filename : ";
  cin >> filename;

  int sec, usec;
  struct timeval start, end;
  gettimeofday(&start, 0);

  for(int round = 0; round <=1; round++) {
    if(round == 0) cout << "FIFO---" << endl;
    else cout << "LRU---" << endl;
    cout << "size   miss    hit     page fault ratio" << endl;

    for (int frame_num = 128; frame_num <= 1024; frame_num *= 2) {
      cout << frame_num << "\t";
      ifstream tracefile;
      tracefile.open(filename);

      unsigned int input;
      int frame_used = 0, add = 1;
      long long miss = 0, hit = 0;
      float ratio = 0;
      node *head = NULL, *tail = NULL;
      for (int i = 0; i < 100; i++) find_page[i] = NULL;

      while (tracefile >> input) {
        page *find = find_page[input % 100];

        while (find != NULL) {
          if (find->num == input) {
            hit++;
            add = 0;
            break;
          }
          find = find->next;
        }

        if (add == 0 && round == 1) {
          if(find->position == tail); //already last
          else if (find->position == head) { //first
            head = head->next;
            head->prev = NULL;
            find->position->prev = tail;
            find->position->next = NULL;
            tail->next = find->position;
            tail = tail->next;
          }
          else {
            tail->next = find->position;
            find->position->next->prev = find->position->prev; //connect next's prev to prev
            find->position->prev->next = find->position->next; //connect prev's next to next
            find->position->prev = tail; //connect find to the last;
            find->position->next = NULL;
            tail = tail->next;
          }
        }

        else if (add == 1) {
          miss++;
          if (frame_used == 0) {
            node *ptr = new node;  // add to frame list
            ptr->num = input;
            ptr->next = NULL;
            ptr->prev = NULL;
            head = ptr;
            tail = ptr;

            page *temp = new page;  // add to hash
            temp->num = input;
            temp->next = find_page[input % 100];
            temp->position = ptr;
            find_page[input % 100] = temp;

            frame_used++;
          } 
          else {
            if (frame_used == frame_num) {  // if frame is fully occupied
              // remove first frame
              unsigned int target = head->num;
              head = head->next;
              page *find = find_page[target % 100],
                  *prev = find_page[target % 100];

              if (find->num == target) {
                find_page[target % 100] = find->next;
                free(find->position);
                free(find);
              } 
              else {
                while (find->next != NULL) {
                  prev = find;
                  find = find->next;
                  if (find->num == target) {
                    prev->next = find->next;
                    free(find->position);
                    free(find);
                    break;
                  }
                }
              }
            } 
            else frame_used++;  // if still have empty frame

            node *ptr = new node;  // add to frame list
            ptr->num = input;
            ptr->next = NULL;
            ptr->prev = tail;
            tail->next = ptr;
            tail = ptr;

            page *temp = new page;  // add to hash
            temp->num = input;
            temp->next = find_page[input % 100];
            temp->position = ptr;
            find_page[input % 100] = temp;
          }
        }

        add = 1;
      }

      tracefile.close();

      ratio = (float)miss / (float)(miss + hit);
      cout << miss << "\t" << hit << "\t" << fixed << setprecision(9) << ratio << endl;
    }
  }

  gettimeofday(&end, 0);
  sec = end.tv_sec - start.tv_sec;
  usec = end.tv_usec - start.tv_usec;
  cout << "total time : " << sec + (usec/1000000.0) << " sec" << endl;
}