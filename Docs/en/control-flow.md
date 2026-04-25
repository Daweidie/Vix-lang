# Control Flow

Control flow statements are used to control the execution order of a program. Vix provides a rich set of control flow structures.

## Table of Contents

- [Conditional Statements](#conditional-statements)
- [while Loop](#while-loop)
- [for Loop](#for-loop)
- [break and continue](#break-and-continue)
- [match Statement](#match-statement)
- [Logical Operations](#logical-operations)

---

## Conditional Statements

### if Statement

```vix
let x = 10

if (x > 5) {
    print("x is greater than 5")
}
```

### if-else Statement

```vix
let x = 10

if (x > 15) {
    print("x is greater than 15")
} else {
    print("x is 15 or less")
}
```

### if-elif-else Statement

```vix
let score = 85

if (score >= 90) {
    print("Grade: A")
} elif (score >= 80) {
    print("Grade: B")
} elif (score >= 70) {
    print("Grade: C")
} elif (score >= 60) {
    print("Grade: D")
} else {
    print("Grade: F")
}
```

### Nested if Statement

```vix
let age = 25
let hasLicense = true

if (age >= 18) {
    if (hasLicense) {
        print("You can drive")
    } else {
        print("You need a license to drive")
    }
} else {
    print("You are too young to drive")
}
```

### Conditional Expressions

The condition can be any expression that returns a boolean value:

```vix
let a = 10
let b = 20

if (a < b and b < 30) {
    print("Both conditions are true")
}

if (a == 10 or b == 10) {
    print("At least one condition is true")
}

if (!(a > b)) {
    print("a is not greater than b")
}
```

---

## while Loop

The while loop repeatedly executes a block of code as long as a condition is true.

### Basic Usage

```vix
let i = 0
while (i < 5) {
    print(i)
    i += 1
}
// Output: 0 1 2 3 4
```

### Infinite Loop

```vix
while (true) {
    // Infinite loop
    // Use break to exit
}
```

### Infinite Loop with Condition

```vix
let running = true
while (running) {
    let input = getInput()
    if (input == "quit") {
        running = false
    }
}
```

### while with User Interaction

```vix
let guess = 0
let target = 42

while (guess != target) {
    guess = toint(input("Guess a number: "))
    if (guess < target) {
        print("Too low!")
    } elif (guess > target) {
        print("Too high!")
    }
}
print("Correct!")
```

---

## for Loop

The for loop is used to iterate over a range or a collection.

### Range Loop

```vix
// Iterate from 1 to 9 (excluding 10)
for (i in 1 .. 10) {
    print(i)
}
// Output: 1 2 3 4 5 6 7 8 9
```

### Loop Starting from Zero

```vix
for (i in 0 .. 5) {
    print(i)
}
// Output: 0 1 2 3 4
```

### Iterating Over an Array

```vix
let arr = [10, 20, 30, 40, 50]

for (i in 0 .. arr.length) {
    print("arr[", i, "] = ", arr[i])
}
```

### Nested Loops

```vix
// Print multiplication table
for (i in 1 .. 10) {
    for (j in 1 .. 10) {
        printf("%4d", i * j)
    }
    print("")
}
```

### Modifying Loop Variable

```vix
for (i in 0 .. 10) {
    if (i % 2 == 0) {
        print(i)  // Only print even numbers
    }
}
```

---

## break and continue

### break Statement

`break` is used to exit a loop immediately:

```vix
let i = 0
while (true) {
    print(i)
    i += 1
    if (i >= 5) {
        break  // Exit loop
    }
}
// Output: 0 1 2 3 4
```

Using in a for loop:

```vix
for (i in 1 .. 100) {
    if (i == 5) {
        break  // Exit when i == 5
    }
    print(i)
}
// Output: 1 2 3 4
```

Finding an element:

```vix
let arr = [3, 7, 2, 9, 5]
let target = 9
let found = -1

for (i in 0 .. arr.length) {
    if (arr[i] == target) {
        found = i
        break  // Exit immediately after finding
    }
}

if (found >= 0) {
    print("Found at index: ", found)
} else {
    print("Not found")
}
```

### continue Statement

`continue` is used to skip the current iteration and proceed to the next loop cycle:

```vix
// Only print odd numbers
for (i in 1 .. 10) {
    if (i % 2 == 0) {
        continue  // Skip even numbers
    }
    print(i)
}
// Output: 1 3 5 7 9
```

Filtering data:

```vix
let numbers = [1, -2, 3, -4, 5, -6]

for (i in 0 .. numbers.length) {
    if (numbers[i] < 0) {
        continue  // Skip negative numbers
    }
    print(numbers[i])
}
// Output: 1 3 5
```

### Combined Usage

```vix
for (i in 0 .. 100) {
    if (i % 3 == 0) {
        continue  // Skip multiples of 3
    }
    if (i > 20) {
        break  // Stop if greater than 20
    }
    print(i)
}
```

---

## match Statement

The match statement is used for pattern matching:

### Basic Usage

```vix
let value = 10

match value {
    5 -> {
        print("value is 5")
    }
    10 -> {
        print("value is 10")
    }
    15 -> {
        print("value is 15")
    }
    _ -> {
        print("value is something else")
    }
}
```

### Matching Enumeration Values

```vix
type State = Running | Stopped | Paused

let state = Running

match state {
    Running -> {
        print("System is running")
    }
    Stopped -> {
        print("System is stopped")
    }
    Paused -> {
        print("System is paused")
    }
}
```

### match with Return Value

```vix
fn describe(n: i32) -> string {
    match n {
        0 -> {
            return "zero"
        }
        1 -> {
            return "one"
        }
        _ -> {
            return "many"
        }
    }
}
```

---

## Logical Operations

### and Operator

Returns true if both conditions are true:

```vix
let age = 25
let hasID = true

if (age >= 18 and hasID) {
    print("You can enter")
}
```

Short-circuit evaluation:

```vix
let x = 0
if (x != 0 and 10 / x > 1) {
    // If x != 0 is false, the subsequent expression is not evaluated
    // Avoids division by zero error
}
```

### or Operator

Returns true if at least one condition is true:

```vix
let day = "Saturday"

if (day == "Saturday" or day == "Sunday") {
    print("It's the weekend!")
}
```

### not Operator

Negates a boolean value:

```vix
let isEmpty = false

if (!isEmpty) {
    print("There are items")
}
```

### Composite Logical Expressions

```vix
let a = 10
let b = 20
let c = 30

if ((a < b and b < c) or a == 0) {
    print("Complex condition is true")
}
```

---

## Control Flow Best Practices

### 1. Avoid Deep Nesting

```vix
// Not recommended
if (condition1) {
    if (condition2) {
        if (condition3) {
            // Too deep
        }
    }
}

// Recommended: use early returns
if (!condition1) {
    return
}
if (!condition2) {
    return
}
if (!condition3) {
    return
}
// Main logic
```

### 2. Use Meaningful Condition Variables

```vix
// Not recommended
if (age >= 18 and hasLicense and !isSuspended) {
    // ...
}

// Recommended
let canDrive = age >= 18 and hasLicense and !isSuspended
if (canDrive) {
    // ...
}
```

### 3. Choose the Right Loop Type

- Use `for` loop to iterate over a known range
- Use `while` loop to handle uncertain number of iterations

### 4. Avoid Infinite Loops

```vix
// Dangerous: no exit condition
while (true) {
    // Forgot break
}

// Safe: explicit exit condition
let running = true
while (running) {
    // ...
    if (shouldStop) {
        running = false
    }
}
```

---

## Examples

### Binary Search

```vix
fn binarySearch(arr: [i32], target: i32, low: i32, high: i32) -> i32 {
    while (low <= high) {
        let mid = (low + high) / 2
        if (arr[mid] == target) {
            return mid
        } elif (arr[mid] < target) {
            low = mid + 1
        } else {
            high = mid - 1
        }
    }
    return -1
}
```

### Bubble Sort

```vix
fn bubbleSort(arr: [i32], size: i32) {
    for (i in 0 .. size - 1) {
        for (j in 0 .. size - i - 1) {
            if (arr[j] > arr[j + 1]) {
                let temp = arr[j]
                arr[j] = arr[j + 1]
                arr[j + 1] = temp
            }
        }
    }
}
```

### Guessing Game

```vix
fn game() -> i32 {
    let target = 42
    let guesses = 0
    
    while (true) {
        let guess = toint(input("Guess: "))
        guesses += 1
        
        if (guess == target) {
            print("Correct! You got it in ", guesses, " guesses!")
            return 0
        } elif (guess < target) {
            print("Too low!")
        } else {
            print("Too high!")
        }
    }
}
```

---

## Next Steps

- [Functions](functions.md) - Function definition and calling
- [Arrays and Lists](syntax.md#arrays-and-lists) - Data structures
- [Pointers](pointers.md) - Pointer operations