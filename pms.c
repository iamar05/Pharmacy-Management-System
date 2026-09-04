#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#endif

//  ANSI COLOR CODES
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define RED     "\033[1;31m"
#define GREEN   "\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE    "\033[1;34m"
#define CYAN    "\033[1;36m"
#define MAGENTA "\033[1;35m"
#define WHITE   "\033[1;37m"

//  CONSTANTS
#define HASH_SIZE 101
#define MAX_NAME  100
#define MAX_MEDS  200

typedef struct Medicine {
    int   id;
    char  name[MAX_NAME];
    int   quantity;
    float price;
    int   exp_day, exp_month, exp_year;  // expiry date
} Medicine;

// ── Hash Table (chaining) ──
typedef struct HashNode {
    Medicine        med;
    struct HashNode *next;
} HashNode;

HashNode *hashTable[HASH_SIZE];

// ── Min-Heap (by expiry date) ──
Medicine heap[MAX_MEDS];
int      heapSize = 0;

// ── Restock Queue (linked list) ──
typedef struct QueueNode {
    char             medName[MAX_NAME];
    int              requestedQty;
    struct QueueNode *next;
} QueueNode;

QueueNode *qFront = NULL;
QueueNode *qRear  = NULL;

//  UTILITY: EXPIRY COMPARISON (returns days since epoch approx)
int expiryKey(int d, int m, int y) {
    return y * 10000 + m * 100 + d;
}

//  HASH TABLE FUNCTIONS
int hashFunc(const char *name) {
    unsigned long hash = 5381;
    while (*name)
        hash = ((hash << 5) + hash) + (unsigned char)(*name++);
    return (int)(hash % HASH_SIZE);
}

void hashInsert(Medicine m) {
    int idx = hashFunc(m.name);
    HashNode *node = (HashNode *)malloc(sizeof(HashNode));
    node->med  = m;
    node->next = hashTable[idx];
    hashTable[idx] = node;
}

Medicine *hashSearch(const char *name) {
    int idx = hashFunc(name);
    HashNode *cur = hashTable[idx];
    while (cur) {
        if (strcmp(cur->med.name, name) == 0) return &cur->med;
        cur = cur->next;
    }
    return NULL;
}

int hashDelete(const char *name) {
    int idx = hashFunc(name);
    HashNode *cur = hashTable[idx], *prev = NULL;
    while (cur) {
        if (strcmp(cur->med.name, name) == 0) {
            if (prev) prev->next = cur->next;
            else hashTable[idx] = cur->next;
            free(cur);
            return 1;
        }
        prev = cur;
        cur  = cur->next;
    }
    return 0;
}
//  MIN-HEAP FUNCTIONS (by expiry date)
void heapSwap(int a, int b) {
    Medicine tmp = heap[a];
    heap[a] = heap[b];
    heap[b] = tmp;
}

void heapifyUp(int i) {
    while (i > 0) {
        int parent = (i - 1) / 2;
        int ki = expiryKey(heap[i].exp_day, heap[i].exp_month, heap[i].exp_year);
        int kp = expiryKey(heap[parent].exp_day, heap[parent].exp_month, heap[parent].exp_year);
        if (ki < kp) { heapSwap(i, parent); i = parent; }
        else break;
    }
}

void heapifyDown(int i) {
    int n = heapSize;
    while (1) {
        int left = 2*i+1, right = 2*i+2, smallest = i;
        if (left  < n && expiryKey(heap[left].exp_day,  heap[left].exp_month,  heap[left].exp_year)  < expiryKey(heap[smallest].exp_day, heap[smallest].exp_month, heap[smallest].exp_year))  smallest = left;
        if (right < n && expiryKey(heap[right].exp_day, heap[right].exp_month, heap[right].exp_year) < expiryKey(heap[smallest].exp_day, heap[smallest].exp_month, heap[smallest].exp_year)) smallest = right;
        if (smallest != i) { heapSwap(i, smallest); i = smallest; }
        else break;
    }
}

void heapInsert(Medicine m) {
    if (heapSize >= MAX_MEDS) { printf(RED "Heap full!\n" RESET); return; }
    heap[heapSize++] = m;
    heapifyUp(heapSize - 1);
}

Medicine heapPeek() { return heap[0]; }

void heapRemoveTop() {
    heap[0] = heap[--heapSize];
    heapifyDown(0);
}
//  QUEUE FUNCTIONS (restock requests)
void enqueue(const char *name, int qty) {
    QueueNode *node = (QueueNode *)malloc(sizeof(QueueNode));
    strncpy(node->medName, name, MAX_NAME);
    node->requestedQty = qty;
    node->next = NULL;
    if (!qRear) { qFront = qRear = node; }
    else { qRear->next = node; qRear = node; }
}

QueueNode *dequeue() {
    if (!qFront) return NULL;
    QueueNode *tmp = qFront;
    qFront = qFront->next;
    if (!qFront) qRear = NULL;
    return tmp;
}

//  UI HELPERS
void clearScreen() {
    printf("\033[2J\033[H");
}

void printLine(char c, int len, const char *color) {
    printf("%s", color);
    for (int i = 0; i < len; i++) putchar(c);
    printf(RESET "\n");
}

void printHeader() {
    clearScreen();
    printLine('=', 58, CYAN);
    printf(WHITE "  ██████╗ ██╗  ██╗ █████╗ ██████╗ ███╗   ███╗ █████╗  \n" RESET);
    printf(WHITE "  ██╔══██╗██║  ██║██╔══██╗██╔══██╗████╗ ████║██╔══██╗ \n" RESET);
    printf(BLUE  "  ██████╔╝███████║███████║██████╔╝██╔████╔██║███████║ \n" RESET);
    printf(BLUE  "  ██╔═══╝ ██╔══██║██╔══██║██╔══██╗██║╚██╔╝██║██╔══██║ \n" RESET);
    printf(RED   "  ██║     ██║  ██║██║  ██║██║  ██║██║ ╚═╝ ██║██║  ██║ \n" RESET);
    printf(RED   "  ╚═╝     ╚═╝  ╚═╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝     ╚═╝╚═╝  ╚═╝ \n" RESET);
    printf(GREEN "        Medicine Stock Management System\n" RESET);
    printLine('=', 58, CYAN);
    printf("\n");
}

void printBox(const char *title, const char *color) {
    int len = strlen(title) + 4;
    printf("%s", color);
    putchar('+');
    for (int i = 0; i < len; i++) putchar('-');
    printf("+\n");
    printf("|  %s  |\n", title);
    putchar('+');
    for (int i = 0; i < len; i++) putchar('-');
    printf("+\n" RESET);
}

// void printMed(Medicine *m) {
//     printf(WHITE "  ┌─────────────────────────────────────┐\n" RESET);
//     printf(WHITE "  │" RESET RED " %-36s" WHITE "│\n" RESET, m->name);
//     printf(WHITE "  ├─────────────────────────────────────┤\n" RESET);
//     printf(WHITE "  │" RESET "  ID       : " GREEN "%-24d" WHITE "│\n" RESET, m->id);
//     printf(WHITE "  │" RESET "  Quantity : " GREEN "%-24d" WHITE "│\n" RESET, m->quantity);
//     printf(WHITE "  │" RESET "  Price    : " GREEN "BDT %-20.2f" WHITE "│\n" RESET, m->price);
//     printf(WHITE "  │" RESET "  Expiry   : " );
//     if (expiryKey(m->exp_day, m->exp_month, m->exp_year) < expiryKey(1,1,2026))
//         printf(RED "%02d/%02d/%04d" RESET "              " WHITE "│\n" RESET, m->exp_day, m->exp_month, m->exp_year);
//     else
//         printf(GREEN "%02d/%02d/%04d" RESET "              " WHITE "│\n" RESET, m->exp_day, m->exp_month, m->exp_year);
//     printf(WHITE "  └─────────────────────────────────────┘\n\n" RESET);
// }

void printMed(Medicine *m) {
    printf(WHITE "  ┌─────────────────────────────────────┐\n" RESET);
    printf(WHITE "  │" RESET RED " %-36s" WHITE "│\n" RESET, m->name);
    printf(WHITE "  ├─────────────────────────────────────┤\n" RESET);
    printf(WHITE "  │" RESET "  ID       : " GREEN "%-24d" WHITE "│\n" RESET, m->id);
    printf(WHITE "  │" RESET "  Quantity : " GREEN "%-24d" WHITE "│\n" RESET, m->quantity);
    printf(WHITE "  │" RESET "  Price    : " GREEN "BDT %-20.2f" WHITE "│\n" RESET, m->price);

    // Fixed alignment: exact width for date formatting
    if (expiryKey(m->exp_day, m->exp_month, m->exp_year) < expiryKey(1, 1, 2026))
        printf(WHITE "  │" RESET "  Expiry   : " RED "%02d/%02d/%04d" RESET "              " WHITE "│\n" RESET, m->exp_day, m->exp_month, m->exp_year);
    else
        printf(WHITE "  │" RESET "  Expiry   : " GREEN "%02d/%02d/%04d" RESET "              " WHITE "│\n" RESET, m->exp_day, m->exp_month, m->exp_year);

    printf(WHITE "  └─────────────────────────────────────┘\n\n" RESET);
}

void pressEnter() {
    printf(YELLOW "\n  Press ENTER to continue..." RESET);
    getchar(); getchar();
}

//  MENU ACTIONS

void addMedicine() {
    printHeader();
    printBox("ADD NEW MEDICINE", GREEN);
    Medicine m;

    printf(CYAN "\n  Enter Medicine ID    : " RESET); scanf("%d", &m.id);
    printf(CYAN "  Enter Medicine Name  : " RESET); scanf(" %[^\n]", m.name);
    printf(CYAN "  Enter Quantity       : " RESET); scanf("%d", &m.quantity);
    printf(CYAN "  Enter Price (BDT)    : " RESET); scanf("%f", &m.price);
    printf(CYAN "  Expiry Date (DD MM YYYY): " RESET);
    scanf("%d %d %d", &m.exp_day, &m.exp_month, &m.exp_year);

    hashInsert(m);
    heapInsert(m);

    printf(GREEN "\n  ✔ Medicine '%s' added successfully!\n" RESET, m.name);
    pressEnter();
}

void searchMedicine() {
    printHeader();
    printBox("SEARCH MEDICINE", BLUE);
    char name[MAX_NAME];
    printf(CYAN "\n  Enter Medicine Name: " RESET);
    scanf(" %[^\n]", name);

    Medicine *m = hashSearch(name);
    if (m) {
        printf(GREEN "\n  ✔ Medicine Found:\n\n" RESET);
        printMed(m);
    } else {
        printf(RED "\n  ✘ Medicine '%s' not found!\n" RESET, name);
    }
    pressEnter();
}


void viewExpiryOrder() {
    printHeader();
    printBox("MEDICINES BY EXPIRY (Soonest First)", YELLOW);

    if (heapSize == 0) {
        printf(RED "\n  No medicines in stock.\n" RESET);
        pressEnter();
        return;
    }

    Medicine tempHeap[MAX_MEDS];
    int tempSize = heapSize;
    memcpy(tempHeap, heap, sizeof(Medicine) * heapSize);

    printf(YELLOW "\n  Showing %d medicine(s) sorted by expiry:\n\n" RESET, heapSize);

    for (int i = 0; i < tempSize - 1; i++) {
        int minIdx = i;
        for (int j = i+1; j < tempSize; j++) {
            if (expiryKey(tempHeap[j].exp_day, tempHeap[j].exp_month, tempHeap[j].exp_year) <
                expiryKey(tempHeap[minIdx].exp_day, tempHeap[minIdx].exp_month, tempHeap[minIdx].exp_year))
                minIdx = j;
        }
        Medicine tmp = tempHeap[i]; tempHeap[i] = tempHeap[minIdx]; tempHeap[minIdx] = tmp;
    }

    for (int i = 0; i < tempSize; i++) {
        // Print header on a separate line before the box
        printf(MAGENTA "  #%d\n" RESET, i + 1);
        printMed(&tempHeap[i]);
    }
    pressEnter();
}

void updateQuantity() {
    printHeader();
    printBox("UPDATE QUANTITY", MAGENTA);
    char name[MAX_NAME];
    int  qty;

    printf(CYAN "\n  Enter Medicine Name : " RESET); scanf(" %[^\n]", name);
    printf(CYAN "  New Quantity        : " RESET); scanf("%d", &qty);

    Medicine *m = hashSearch(name);
    if (m) {
        m->quantity = qty;
        // Update in heap too
        for (int i = 0; i < heapSize; i++) {
            if (strcmp(heap[i].name, name) == 0) {
                heap[i].quantity = qty;
                break;
            }
        }
        printf(GREEN "\n  ✔ Quantity updated to %d for '%s'\n" RESET, qty, name);
    } else {
        printf(RED "\n  ✘ Medicine not found!\n" RESET);
    }
    pressEnter();
}

void deleteMedicine() {
    printHeader();
    printBox("DELETE MEDICINE", RED);
    char name[MAX_NAME];
    printf(CYAN "\n  Enter Medicine Name to Delete: " RESET);
    scanf(" %[^\n]", name);

    int deleted = hashDelete(name);
    if (deleted) {
        // Remove from heap
        for (int i = 0; i < heapSize; i++) {
            if (strcmp(heap[i].name, name) == 0) {
                heap[i] = heap[--heapSize];
                heapifyDown(i);
                break;
            }
        }
        printf(GREEN "\n  ✔ Medicine '%s' deleted.\n" RESET, name);
    } else {
        printf(RED "\n  ✘ Medicine not found!\n" RESET);
    }
    pressEnter();
}

void requestRestock() {
    printHeader();
    printBox("REQUEST RESTOCK", YELLOW);
    char name[MAX_NAME];
    int  qty;

    printf(CYAN "\n  Medicine Name     : " RESET); scanf(" %[^\n]", name);
    printf(CYAN "  Requested Quantity: " RESET); scanf("%d", &qty);

    enqueue(name, qty);
    printf(GREEN "\n  ✔ Restock request for '%s' (%d units) added to queue.\n" RESET, name, qty);
    pressEnter();
}

void processRestock() {
    printHeader();
    printBox("PROCESS RESTOCK QUEUE", GREEN);

    if (!qFront) {
        printf(RED "\n  No pending restock requests.\n" RESET);
        pressEnter();
        return;
    }

    printf(YELLOW "\n  Processing next restock request...\n\n" RESET);
    QueueNode *node = dequeue();

    printf(WHITE "  ┌─────────────────────────────────────┐\n" RESET);
    printf(WHITE "  │" RESET YELLOW " %-36s" WHITE "│\n" RESET, "Restock Request");
    printf(WHITE "  ├─────────────────────────────────────┤\n" RESET);
    printf(WHITE "  │" RESET "  Medicine : " GREEN "%-24s" WHITE "│\n" RESET, node->medName);
    printf(WHITE "  │" RESET "  Quantity : " GREEN "%-24d" WHITE "│\n" RESET, node->requestedQty);
    printf(WHITE "  └─────────────────────────────────────┘\n\n" RESET);

    // Update stock if medicine exists
    Medicine *m = hashSearch(node->medName);
    if (m) {
        m->quantity += node->requestedQty;
        for (int i = 0; i < heapSize; i++) {
            if (strcmp(heap[i].name, node->medName) == 0) {
                heap[i].quantity = m->quantity;
                break;
            }
        }
        printf(GREEN "  ✔ Stock updated! New quantity: %d\n" RESET, m->quantity);
    } else {
        printf(RED "  ✘ Medicine not found in system. Request discarded.\n" RESET);
    }

    free(node);

    // Show remaining queue
    int count = 0;
    QueueNode *cur = qFront;
    while (cur) { count++; cur = cur->next; }
    printf(YELLOW "\n  Remaining restock requests in queue: %d\n" RESET, count);
    pressEnter();
}

void viewRestockQueue() {
    printHeader();
    printBox("PENDING RESTOCK QUEUE", CYAN);

    if (!qFront) {
        printf(RED "\n  Queue is empty.\n" RESET);
        pressEnter();
        return;
    }

    QueueNode *cur = qFront;
    int i = 1;
    printf(CYAN "\n  %-4s %-30s %-10s\n" RESET, "No.", "Medicine Name", "Qty Requested");
    printLine('-', 50, CYAN);
    while (cur) {
        printf("  %-4d " GREEN "%-30s" RESET " %-10d\n", i++, cur->medName, cur->requestedQty);
        cur = cur->next;
    }
    pressEnter();
}

void viewAllMedicines() {
    printHeader();
    printBox("ALL MEDICINES IN STOCK", CYAN);

    int found = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        HashNode *cur = hashTable[i];
        while (cur) {
            printMed(&cur->med);
            found++;
            cur = cur->next;
        }
    }
    if (!found) printf(RED "\n  No medicines in stock.\n" RESET);
    pressEnter();
}


void checkExpiringSoon() {

    printBox("EXPIRING SOON (Top 5)", RED);

    if (heapSize == 0) {
        printf(RED "\n  No medicines in stock.\n" RESET);
        pressEnter();
        return;
    }

    printf(RED "\n  Medicines expiring soonest:\n\n" RESET);
    int show = heapSize < 5 ? heapSize : 5;

    Medicine tempHeap[MAX_MEDS];
    int tempSize = heapSize;
    memcpy(tempHeap, heap, sizeof(Medicine) * heapSize);
    for (int i = 0; i < tempSize - 1; i++) {
        int minIdx = i;
        for (int j = i+1; j < tempSize; j++) {
            if (expiryKey(tempHeap[j].exp_day, tempHeap[j].exp_month, tempHeap[j].exp_year) <
                expiryKey(tempHeap[minIdx].exp_day, tempHeap[minIdx].exp_month, tempHeap[minIdx].exp_year))
                minIdx = j;
        }
        Medicine tmp = tempHeap[i]; tempHeap[i] = tempHeap[minIdx]; tempHeap[minIdx] = tmp;
    }

    for (int i = 0; i < show; i++) {
        // Print warning prefix on a separate line before the box
        printf(RED "  ⚠  Warning: Expiring Soon\n" RESET);
        printMed(&tempHeap[i]);
    }
    pressEnter();
}

//  LOAD SAMPLE DATA
void loadSampleData() {
    Medicine samples[] = {
        {1, "Paracetamol 500mg",  200, 5.00,  10, 6,  2026},
        {2, "Amoxicillin 250mg",  150, 12.50, 15, 3,  2026},
        {3, "Omeprazole 20mg",    80,  18.00, 20, 12, 2027},
        {4, "Metformin 500mg",    60,  8.00,  5,  2,  2026},
        {5, "Cetirizine 10mg",    120, 6.50,  1,  9,  2027},
    };
    for (int i = 0; i < 5; i++) {
        hashInsert(samples[i]);
        heapInsert(samples[i]);
    }
}

//  MAIN MENU
int main() {
    // --- ADD THIS BLOCK FOR WINDOWS SUPPORT ---
    #ifdef _WIN32
    // Set console code page to UTF-8
    SetConsoleOutputCP(CP_UTF8);

    // Enable ANSI escape codes
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, dwMode);
        }
    }
    #endif
    // ------------------------------------------

    memset(hashTable, 0, sizeof(hashTable));
    loadSampleData();

    // ... rest of your code continues here ...

    int choice;
    do {
        printHeader();
        printf(WHITE "  Data Structures Used:" RESET
               GREEN " Hash Table" RESET " | "
               YELLOW "Min-Heap" RESET " | "
               CYAN "Queue\n\n" RESET);

        printLine('-', 58, CYAN);
        printf(GREEN  "  [1]" RESET " Add New Medicine\n");
        printf(BLUE   "  [2]" RESET " Search Medicine by Name\n");
        printf(CYAN   "  [3]" RESET " View All Medicines\n");
        printf(YELLOW "  [4]" RESET " View Medicines by Expiry Order\n");
        printf(RED    "  [5]" RESET " Check Expiring Soon (Top 5)\n");
        printf(MAGENTA"  [6]" RESET " Update Medicine Quantity\n");
        printf(RED    "  [7]" RESET " Delete Medicine\n");
        printf(YELLOW "  [8]" RESET " Request Restock\n");
        printf(GREEN  "  [9]" RESET " Process Restock Queue\n");
        printf(CYAN   " [10]" RESET " View Restock Queue\n");
        printf(RED    "  [0]" RESET " Exit\n");
        printLine('-', 58, CYAN);

        printf(WHITE "\n  Enter your choice: " RESET);
        scanf("%d", &choice);

        switch (choice) {
            case 1:  addMedicine();      break;
            case 2:  searchMedicine();   break;
            case 3:  viewAllMedicines(); break;
            case 4:  viewExpiryOrder();  break;
            case 5:  checkExpiringSoon();break;
            case 6:  updateQuantity();   break;
            case 7:  deleteMedicine();   break;
            case 8:  requestRestock();   break;
            case 9:  processRestock();   break;
            case 10: viewRestockQueue(); break;
            case 0:
                printHeader();
                printf(GREEN "  Thank you for using Pharma Stock System!\n\n" RESET);
                break;
            default:
                printf(RED "\n  Invalid choice! Try again.\n" RESET);
                pressEnter();
        }
    } while (choice != 0);

    return 0;
}
