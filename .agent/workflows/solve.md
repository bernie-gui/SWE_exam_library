---
description: Workflow to solve exams
---

# Workflow: Solve Exam Problem

**Trigger:** `/solve`
**Context:** User provides a PDF of the exam problem.

## Step 1: Learn the "Exam Style"

1.  **Review the Toolkit:** Read the header files in `/include`.
    - **Rule:** These are the primary tools.

2.  **Review the Structure:** Read 3-4 random `.cpp` files from `/examples`.
    - **Goal:** Observe the coding pattern. How is `main()` structured? How are the custom headers included?
    - **Note:** The filenames are dates; just pick a few to understand the general boilerplate.

## Step 2: Analyze the Request

1.  **Read the PDF:** Identify the specific algorithm or problem asked in the exam prompt.
2.  **Map to Library:** Determine which classes from `/include` are required to solve this specific problem.

## Step 3: Implement `main.cpp`

1.  **Write the Solution:**
    - **Primary Logic:** Use the custom classes from `/include` to handle the core data structures/logic required by the exam.
    - **Support Logic:** You are free to use standard C++ (`<iostream>`, `<algorithm>`, etc.) for input/output, loops, or auxiliary tasks not covered by the custom library.
    - **Structure:** Mimic the clean `main()` structure you saw in the examples.
    - **Don't add comments, if you do, make them all lowercase**

2.  **Check:** Ensure you haven't re-implemented something that already exists in `/include`.
