# Data Structures Analysis Report

Below is a comprehensive breakdown of the data structures utilized in the Mentoring Management System. It details the purpose, operations, and time complexities of each structure.

## Quick Reference Table

| Data Structure | Entity Stored | Why it is Used (The "Why") | Advantages | Key Operations | Time Complexity |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Binary Search Tree (BST)** | **Users**<br>*(Key: Username)* | To quickly authenticate users logging into the system based on their username. | Faster lookups than a list. Halves the search area at every step when searching by username. | `Insert`<br>`Search (by Username)`<br>`Search (by Entity ID)` | **O(log N)**<br>**O(log N)**<br>**O(N)** |
| **Binary Search Tree (BST)** | **Mentees**<br>*(Key: Roll No.)* | To quickly fetch a mentee's profile or check their details using their Roll Number. | Easy to keep records sorted by Roll Number automatically without running a sorting algorithm. | `Insert`<br>`Search (by Roll No)`<br>`Delete Mentee` | **O(log N)**<br>**O(log N)**<br>**O(log N)** |
| **Stack (LIFO)**<br>*(Linked List)* | **Meetings** | To display the most recently scheduled or updated meetings at the very top of the dashboard. | Guarantees that the newest and most relevant items are accessed first without needing to sort them. | `Push (Add Meeting)`<br>`Traversal (View All)` | **O(1)**<br>**O(M)** |
| **Queue (FIFO)**<br>*(Linked List)* | **Requests** | To handle meeting requests fairly. The first student to ask for a meeting is the first one shown to the mentor. | Prevents older requests from getting buried and ignored at the bottom of a list. | `Enqueue (Add Request)`<br>`Dequeue / Process`<br>`Traversal (View All)` | **O(1)**<br>**O(1)**<br>**O(R)** |
| **Singly Linked List** | **Notes** | To simply log text notes made by mentors over time sequentially. | Simple memory management. Elements can be dynamically added to the end without shifting data. | `Append (Add Note)`<br>`Traversal (View All)` | **O(1)**<br>**O(L)** |

*(Note: `N` = Total Users/Mentees, `M` = Total Meetings, `R` = Total Requests, `L` = Total Notes. O(log N) assumes the tree is reasonably balanced).*

---

## Detailed Breakdown of Operations

### 1. Binary Search Trees (Users & Mentees)
**How it works:** Every time a node is added, the system checks the key (e.g. Roll Number). If it's smaller, it goes to the left arm; if larger, to the right arm.
*   **Insert — `O(log N)`:** Traversing down the arms to find the correct empty spot.
*   **Search by Key — `O(log N)`:** Traversing down the arms to find the exact match. Extremely fast.
*   **Search by non-Key (Entity ID) — `O(N)`:** The tree is not sorted by Entity ID, forcing the system to visit every single node (Pre-order Traversal) to find a match.
*   **Delete — `O(log N)`:** Finding the node and replacing it with its "In-order Successor" (the smallest item in its right subtree) to keep the tree perfectly sorted.

### 2. Stack / LIFO (Meetings)
**How it works:** A Linked List where the system only ever interacts with the **Head** (the Top) of the list.
*   **Push / Add — `O(1)`:** Creates a new Meeting box, points it to the current Head, and declares the new box as the new Head. It takes 1 step regardless of how many meetings exist.
*   **Traversal / View — `O(M)`:** Starts at the Head (newest) and walks down the chain to the end.

### 3. Queue / FIFO (Requests)
**How it works:** A Linked List where the system adds to the **Tail** (the back of the line) and removes from the **Head** (the front of the line).
*   **Enqueue / Add — `O(1)`:** Because the system keeps a dedicated `tail` pointer, adding a new request to the back of the line happens instantly.
*   **Dequeue / Remove — `O(1)`:** Snapping off the front node and moving the `head` pointer to the second person in line.

### 4. Linked List (Notes)
**How it works:** A standard one-way chain of boxes.
*   **Append — `O(1)`:** Similar to the queue, it uses a `tail` pointer during file loading to attach items to the end instantly.
