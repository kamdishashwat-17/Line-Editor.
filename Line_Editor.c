#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>

#define MAX_LINES 25
#define MAX_LINE_LENGTH 256
#define UNDO_LIMIT 3
#define MAX_ENTRIES 25
#define MAX_ENTRY_LENGTH 256
#define HISTORY_LIMIT 3

using namespace std;

vector<string> lines;
vector<string> undo_operations;
char *text_buffer[MAX_ENTRIES];
int total_lines = 0;
char *history_stack[HISTORY_LIMIT];
int history_top = -1;

void addUndoOperation(const string &operation) {
    if (undo_operations.size() >= UNDO_LIMIT) {
        undo_operations.erase(undo_operations.begin());
    }
    undo_operations.push_back(operation);
}

void performUndo() {
    if (undo_operations.empty()) {
        cout << "No operations to undo.\n";
        return;
    }
    string lastOperation = undo_operations.back();
    undo_operations.pop_back();
    // Implement undo logic based on lastOperation
    cout << "Undid operation: " << lastOperation << "\n";
}

void readFile(const string &filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file.\n";
        return;
    }
    string line;
    lines.clear();
    while (getline(file, line)) {
        lines.push_back(line);
    }
    file.close();
}

void writeFile(const string &filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Error opening file.\n";
        return;
    }
    for (const string &line : lines) {
        file << line << "\n";
    }
    file.close();
}

void addLine(const string &line, int position) {
    if (position < 0 || position > lines.size()) {
        cout << "Invalid position.\n";
        return;
    }
    lines.insert(lines.begin() + position, line);
    addUndoOperation("Add Line");
}

void removeLine(int position) {
    if (position < 0 || position >= lines.size()) {
        cout << "Invalid position.\n";
        return;
    }
    lines.erase(lines.begin() + position);
    addUndoOperation("Remove Line");
}

int findWord(const string &word) {
    for (int i = 0; i < lines.size(); ++i) {
        if (lines[i].find(word) != string::npos) {
            return i;
        }
    }
    return -1;
}

void substituteWord(const string &oldWord, const string &newWord) {
    for (string &line : lines) {
        size_t pos = 0;
        while ((pos = line.find(oldWord, pos)) != string::npos) {
            line.replace(pos, oldWord.length(), newWord);
            pos += newWord.length();
        }
    }
    addUndoOperation("Substitute Word");
}

void displayLines() {
    for (int i = 0; i < lines.size(); ++i) {
        cout << i + 1 << ": " << lines[i] << "\n";
    }
}

void clearLines() {
    lines.clear();
    addUndoOperation("Clear Lines");
}

void removeWordAtPosition(int lineIndex, int position) {
    if (lineIndex < 0 || lineIndex >= lines.size()) {
        cout << "Invalid line index.\n";
        return;
    }

    string &line = lines[lineIndex];
    size_t start = position;
    while (start < line.size() && line[start] != ' ' && line[start] != '\n') {
        ++start;
    }

    line.erase(position, start - position);
    addUndoOperation("Remove Word");
    cout << "Word removed from line " << lineIndex + 1 << ".\n";
}

void substituteWordAtPosition(int lineIndex, int position, const string &newWord) {
    if (lineIndex < 0 || lineIndex >= lines.size()) {
        cout << "Invalid line index.\n";
        return;
    }

    string &line = lines[lineIndex];
    size_t start = position;
    while (start < line.size() && line[start] != ' ' && line[start] != '\n') {
        ++start;
    }

    line.replace(position, start - position, newWord);
    addUndoOperation("Substitute Word");
    cout << "Word replaced in line " << lineIndex + 1 << ".\n";
}

void initialize_buffer() {
    for (int i = 0; i < MAX_ENTRIES; ++i) {
        text_buffer[i] = NULL;
    }
}

void push_history(const char *operation) {
    if (history_top >= HISTORY_LIMIT - 1) {
        free(history_stack[0]);
        for (int i = 1; i <= history_top; ++i) {
            history_stack[i - 1] = history_stack[i];
        }
        --history_top;
    }
    history_stack[++history_top] = strdup(operation);
}

void undo_last_operation() {
    if (history_top < 0) {
        printf("No operations to undo.\n");
        return;
    }
    printf("Undid operation: %s\n", history_stack[history_top]);
    free(history_stack[history_top--]);
}

void load_text_file(FILE *file) {
    char line[MAX_ENTRY_LENGTH];
    total_lines = 0;
    while (fgets(line, sizeof(line), file) && total_lines < MAX_ENTRIES) {
        text_buffer[total_lines++] = strdup(line);
    }
}

void save_text_file(const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printf("Error opening file.\n");
        return;
    }
    for (int i = 0; i < total_lines; ++i) {
        fprintf(file, "%s", text_buffer[i]);
    }
    fclose(file);
}

void insert_text_line(const char *line, int pos) {
    if (pos < 0 || pos > total_lines || total_lines >= MAX_ENTRIES) {
        printf("Invalid position.\n");
        return;
    }
    for (int i = total_lines; i > pos; --i) {
        text_buffer[i] = text_buffer[i - 1];
    }
    text_buffer[pos] = strdup(line);
    ++total_lines;
    push_history("Insert Line");
}

void delete_text_line(int pos) {
    if (pos < 0 || pos >= total_lines) {
        printf("Invalid position.\n");
        return;
    }
    free(text_buffer[pos]);
    for (int i = pos; i < total_lines - 1; ++i) {
        text_buffer[i] = text_buffer[i + 1];
    }
    --total_lines;
    push_history("Delete Line");
}

int main() {
    int option;
    string input;
    string searchWord;
    string filename;
    int position;
    string line;
    string oldWord, newWord;

    while (true) {
        cout << "1. Read File\n";
        cout << "2. Write File\n";
        cout << "3. Add Line\n";
        cout << "4. Remove Line\n";
        cout << "5. Find Word\n";
        cout << "6. Substitute Word\n";
        cout << "7. Display Lines\n";
        cout << "8. Clear Lines\n";
        cout << "9. Remove Word At Position\n";
        cout << "10. Substitute Word At Position\n";
        cout << "11. Undo\n";
        cout << "12. Exit\n";
        cout << "Enter your option: ";
        cin >> option;

        switch (option) {
            case 1:
                cout << "Enter filename: ";
                cin >> filename;
                readFile(filename);
                break;
            case 2:
                cout << "Enter filename: ";
                cin >> filename;
                writeFile(filename);
                break;
            case 3:
                cout << "Enter line: ";
                cin.ignore();
                getline(cin, line);
                cout << "Enter position: ";
                cin >> position;
                addLine(line, position);
                break;
            case 4:
                cout << "Enter position: ";
                cin >> position;
                removeLine(position);
                break;
            case 5:
                cout << "Enter word to find: ";
                cin >> searchWord;
                position = findWord(searchWord);
                if (position != -1) {
                    cout << "Word found at line " << position + 1 << "\n";
                } else {
                    cout << "Word not found\n";
                }
                break;
            case 6:
                cout << "Enter old word: ";
                cin >> oldWord;
                cout << "Enter new word: ";
                cin >> newWord;
                substituteWord(oldWord, newWord);
                break;
            case 7:
                displayLines();
                break;
            case 8:
                clearLines();
                break;
            case 9:
                cout << "Enter line index: ";
                cin >> position;
                cout << "Enter position: ";
                cin >> position;
                removeWordAtPosition(position, position);
                break;
            case 10:
                cout << "Enter line index: ";
                cin >> position;
                cout << "Enter position: ";
                cin >> position;
                cout << "Enter new word: ";
                cin >> newWord;
                substituteWordAtPosition(position, position, newWord);
                break;
            case 11:
                performUndo();
                break;
            case 12:
                return 0;
            default:
                cout << "Invalid option\n";
                break;
        }
    }

    return 0;
}