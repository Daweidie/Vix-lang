# Structs

Structs are composite types in Vix used to organize related data. They allow you to combine multiple values of different types into a single meaningful unit.

## Table of Contents

- [Defining Structs](#defining-structs)
- [Creating Instances](#creating-instances)
- [Accessing Fields](#accessing-fields)
- [Modifying Fields](#modifying-fields)
- [Nested Structs](#nested-structs)
- [Struct Methods](#struct-methods)
- [Generic Structs](#generic-structs)
- [Structs as Parameters](#structs-as-parameters)
- [Best Practices](#best-practices)

---

## Defining Structs

### Basic Syntax

```vix
struct StructName {
    field1: type1,
    field2: type2,
    field3: type3
}
```

### Examples

```vix
struct Person {
    name: string,
    age: i32,
    height: f64
}

struct Book {
    title: string,
    author: string,
    year: i32,
    pages: i32
}

struct Point {
    x: f64,
    y: f64
}
```

### Public Structs

Use the `pub` keyword to make a struct exportable:

```vix
pub struct Person {
    name: string,
    age: i32
}
```

---

## Creating Instances

### Using Initializer Lists

```vix
let person = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

let book = Book {
    title: "The Vix Guide",
    author: "Vix Team",
    year: 2024,
    pages: 350
}
```

### Field-by-field Assignment

```vix
let person: Person {}
person.name = "Bob"
person.age = 30
person.height = 6.0
```

### Using Variables for Initialization

```vix
let name = "Charlie"
let age = 28
let height = 5.9

let person = Person {
    name: name,
    age: age,
    height: height
}
```

---

## Accessing Fields

Use the `.` operator to access struct fields:

```vix
let person = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

print(person.name)     // "Alice"
print(person.age)      // 25
print(person.height)   // 5.7
```

### Chained Access

```vix
struct Address {
    street: string,
    city: string,
    zip: string
}

struct Person {
    name: string,
    address: Address
}

let person = Person {
    name: "Alice",
    address: Address {
        street: "123 Main St",
        city: "Anytown",
        zip: "12345"
    }
}

print(person.address.city)    // "Anytown"
print(person.address.street)  // "123 Main St"
```

---

## Modifying Fields

### Mutable Instances

Use `mut` to declare a mutable struct instance:

```vix
mut person = Person {
    name: "Alice",
    age: 25,
    height: 5.7
}

person.age = 26           // Modify age
person.height = 5.8       // Modify height

print(person.age)         // 26
print(person.height)      // 5.8
```

### Calculating and Modifying Fields

```vix
mut rect = Rectangle {
    width: 10.0,
    height: 5.0
}

// Modify field values
rect.width = rect.width * 2
rect.height = rect.height + 2.0

print(rect.width)   // 20.0
print(rect.height)  // 7.0
```

---

## Nested Structs

Structs can contain other structs as fields:

```vix
struct Date {
    year: i32,
    month: i32,
    day: i32
}

struct Event {
    name: string,
    date: Date,
    location: string
}

let event = Event {
    name: "Vix Conference",
    date: Date {
        year: 2024,
        month: 6,
        day: 15
    },
    location: "San Francisco"
}

print(event.name)           // "Vix Conference"
print(event.date.year)      // 2024
print(event.date.month)     // 6
print(event.location)       // "San Francisco"
```

### Deep Nesting

```vix
struct Country {
    name: string,
    code: string
}

struct City {
    name: string,
    country: Country
}

struct Address {
    street: string,
    city: City
}

struct Person {
    name: string,
    address: Address
}

let person = Person {
    name: "John",
    address: Address {
        street: "123 Main St",
        city: City {
            name: "New York",
            country: Country {
                name: "United States",
                code: "US"
            }
        }
    }
}

print(person.address.city.country.name)  // "United States"
```

---

## Struct Methods

Vix uses functions to operate on structs:

### Basic Method Pattern

```vix
struct Rectangle {
    width: f64,
    height: f64
}

// Calculate area
fn area(rect: Rectangle) -> f64 {
    return rect.width * rect.height
}

// Calculate perimeter
fn perimeter(rect: Rectangle) -> f64 {
    return 2.0 * (rect.width + rect.height)
}

// Usage
let rect = Rectangle { width: 10.0, height: 5.0 }
print(area(rect))       // 50.0
print(perimeter(rect))  // 30.0
```

### Methods that Modify a Struct

```vix
struct Counter {
    value: i32
}

fn increment(counter: &Counter) {
    @counter.value = @counter.value + 1
}

fn reset(counter: &Counter) {
    @counter.value = 0
}

// Usage
mut counter = Counter { value: 0 }
increment(&counter)
print(counter.value)  // 1
reset(&counter)
print(counter.value)  // 0
```

### Factory Functions

```vix
struct Point {
    x: f64,
    y: f64
}

fn newPoint(x: f64, y: f64) -> Point {
    return Point { x: x, y: y }
}

fn origin() -> Point {
    return Point { x: 0.0, y: 0.0 }
}

// Usage
let p1 = newPoint(3.0, 4.0)
let p2 = origin()
```

---

## Generic Structs

Structs can use generic parameters:

### Defining Generic Structs

```vix
struct Box:[T] {
    value: T
}

struct Pair:[T, U] {
    first: T,
    second: U
}
```

### Instantiating Generic Structs

```vix
// Specify concrete types
let intBox = Box:[i32]{ value: 42 }
let floatBox = Box:[f64]{ value: 3.14 }
let strBox = Box:[string]{ value: "Hello" }

// Multiple generic parameters
let pair = Pair:[i32, string]{
    first: 1,
    second: "one"
}
```

### Methods for Generic Structs

```vix
struct Box:[T] {
    value: T
}

fn get:[T](box: Box:[T]) -> T {
    return box.value
}

fn set:[T](box: &Box:[T], newValue: T) {
    @box.value = newValue
}

// Usage
let box = Box:[i32]{ value: 42 }
print(get:[i32](box))  // 42
```

---

## Structs as Parameters

### Pass-by-Value

```vix
fn printPerson(p: Person) {
    print("Name: ", p.name)
    print("Age: ", p.age)
}

let person = Person { name: "Alice", age: 25 }
printPerson(person)
```

### Pass-by-Pointer

```vix
fn updateAge(p: &Person, newAge: i32) {
    @p.age = newAge
}

mut person = Person { name: "Alice", age: 25 }
updateAge(&person, 26)
print(person.age)  // 26
```

### Returning a Struct

```vix
fn createPerson(name: string, age: i32) -> Person {
    return Person {
        name: name,
        age: age,
        height: 5.7
    }
}

let person = createPerson("Bob", 30)
```

---

## Best Practices

### 1. Meaningful Naming

```vix
// Recommended: descriptive naming
struct UserProfile {
    username: string,
    email: string,
    createdAt: i64
}

// Not recommended: abbreviated or unclear naming
struct UP {
    u: string,
    e: string,
    c: i64
}
```

### 2. Organize Related Fields

```vix
// Recommended: organize related fields together
struct Address {
    street: string,
    city: string,
    state: string,
    zip: string,
    country: string
}

struct User {
    name: string,
    email: string,
    address: Address  // Use nested struct
}
```

### 3. Use Factory Functions

```vix
// Recommended: provide a convenient method to create instances
fn newUser(name: string, email: string) -> User {
    return User {
        name: name,
        email: email,
        active: true
    }
}
```

### 4. Prefer Immutability

```vix
// Use immutable by default
let person = Person { name: "Alice", age: 25 }

// Use mut only when modification is necessary
mut counter = Counter { value: 0 }
```

---

## Examples

### Graphics Library

```vix
struct Point {
    x: f64,
    y: f64
}

struct Size {
    width: f64,
    height: f64
}

struct Rectangle {
    origin: Point,
    size: Size
}

fn area(rect: Rectangle) -> f64 {
    return rect.size.width * rect.size.height
}

fn containsPoint(rect: Rectangle, p: Point) -> bool {
    return p.x >= rect.origin.x and
           p.x <= rect.origin.x + rect.size.width and
           p.y >= rect.origin.y and
           p.y <= rect.origin.y + rect.size.height
}
```

### Linked List Node

```vix
struct Node:[T] {
    value: T,
    next: &Node:[T]
}

fn createNode:[T](value: T) -> Node:[T] {
    return Node:[T]{
        value: value,
        next: nil
    }
}
```

---

## Next Steps

- [Functions](functions.md) - Function definitions
- [Generics](types.md#generic-types) - Generic types
- [Pointers](pointers.md) - Pointer operations