# C++ Style Guide

## 1. Purpose

This style guide defines the coding conventions the MercuryCAN project written in C++23.

The rules are intentionally explicit and deterministic so that they can be followed consistently by both human developers and AI coding assistants.

When rules conflict, apply them in the following order:

1. Correctness
2. Safety
3. Readability
4. Maintainability
5. Performance
6. Consistency

Do not optimize for performance without evidence that performance is relevant.

---

## 2. General Principles

### 2.1 One Responsibility per Method

* Every method must do exactly one logical thing.
* If a method performs multiple independent tasks, split it into multiple methods.
* If the individual operations are logically independent, the implementation should be split into appropriately named methods.

### 2.2 Method Length

* A method must contain at most **20 lines**.
* Prefer methods that are significantly shorter than 20 lines.
* If a method exceeds 20 lines, split it into smaller methods.
* Do not circumvent this rule by creating meaningless helper methods.
* A helper method must represent a meaningful logical operation.
* The 20-line limit applies to the method body and excludes the method signature.

### 2.3 Avoid Deep Nesting

Avoid more than three levels of nesting.

Prefer early returns:

```cpp
if (!IsValid())
    return;

if (!HasPermission())
    return;

Process();
```

over deeply nested code:

```cpp
if (IsValid()) {
    if (HasPermission()) {
        Process();
    }
}
```

---

## 2.4 Doxygen Documentation

All code elements must be documented with Doxygen.

This includes classes, structs, enums, methods, functions, member variables, and
other relevant declarations that are part of the implementation.

Documentation should describe the purpose, behavior, and relevant constraints of
the element.

The documentation requirement applies to all code that is meant to be read,
maintained, or reused by other developers, regardless of visibility.

---

# 3. Naming

## 3.1 General Rules

Names must describe their purpose.

Avoid abbreviations unless they are universally understood.

Prefer:

```cpp
connectionCount
maximumRetries
userRepository
```

over:

```cpp
connCnt
maxRet
usrRepo
```

Names must be in English.

---

## 3.2 Classes and Structs

Use `PascalCase`.

```cpp
class UserRepository;
struct ConnectionConfig;
```

---

## 3.3 Methods

Use `camelCase` for methods.

Method names should describe an action.

```cpp
UserInfo loadUser();
double calculateDistance();
void sendMessage();
bool validate();
bool checkPermission();
```

---

## 3.4 Variables

Use `camelCase`.

```cpp
userName
connectionCount
maximumRetries
```

If a member variable is private, it should begin with an underscore.

```cpp
class House
{
private:
    const std::string _address;
}
```

---

## 3.5 Constants

Use `camelCase` for named constants.

```cpp
constexpr int maximumRetries = 3;
```

---

## 3.6 Avoid Generic Names

Avoid names such as:

```cpp
data
value
object
thing
temp
foo
bar
```

unless their meaning is genuinely clear from the local context.

Prefer:

```cpp
userData
calculatedValue
temporaryResult
```

---

# 4. Types and `const`

## 4.1 Prefer `const`

Use `const` whenever a value does not need to be modified.

```cpp
const UserInfo user = loadUser();
```

Prefer:

```cpp
void process(const User& user);
```

over:

```cpp
void process(User& user);
```

if the method does not modify the user.

---

# 5. Parameters

## 5.1 Avoid Unnecessary Parameters

A method should receive only the data it actually needs.

Avoid passing large objects when only a small part is required.

Prefer:

```cpp
void setUserName(std::string userName);
```

over:

```cpp
void setUser(const User& user);
```

if only the name is required.

---

## 5.2 Parameter Count

Prefer no more than four parameters.

If a method requires more parameters, consider introducing a meaningful configuration
or parameter type.

---

# 6. Return Values

Prefer returning values over modifying output parameters.

Prefer:

```cpp
User loadUser(UserId id);
```

over:

```cpp
void loadUser(UserId id, User& user);
```

Use output parameters only when there is a clear technical reason.

---

# 7. Loops

Use range-based `for` loops when possible.

Prefer:

```cpp
for (const auto& item : items)
    process(item);
```

over:

```cpp
for (std::size_t i = 0; i < items.size(); ++i)
    process(items[i]);
```

Use an indexed loop when the index is actually required.

---

# 8. Conditions

Keep conditions simple.

If a condition becomes difficult to understand, extract it into a named method
or variable.

Use Yoda conditions for comparisons with literals or enum values.

Prefer:

```cpp
if (3 == retryCount)
    return;

if (ConnectionState::Connected == state)
    return;
```

over:

```cpp
if (retryCount == 3)
    return;

if (state == ConnectionState::Connected)
    return;
```

Do not force Yoda style onto boolean predicates or already readable checks.

For complex boolean logic, prefer a named variable as shown below:

```cpp
const bool canProcess = user.IsActive() &&
                        user.HasPermission() &&
                        !user.IsBlocked();

if (!canProcess)
    return;
```

---

# 9. Switch Statements

Every `switch` must explicitly handle all relevant cases.

Never use `default` case to detect a missing case.

Avoid accidental fallthrough.

If fallthrough is intentional, make it explicit and documented.

---

# 10. Classes

Classes should have a clear responsibility.

Avoid classes that act as containers for unrelated functionality.

Prefer composition over inheritance unless inheritance represents a genuine
"is-a" relationship.

Keep the public interface as small as possible.

---

# 11. Header Files

Header files should contain only what is necessary.

Prefer forward declarations where appropriate.

Include what you use.

Do not rely on transitive includes.

Example:

```cpp
// user.hpp
#ifndef __USER_HPP__
#define __USER_HPP__

class AuthorizationInfo;

class User
{
public:
    bool checkAdminRights() const;

private:
    const AuthorizationInfo* _authorization = nullptr;
};

#endif //__USER_HPP__
```

---

# 12. Include Order

Use a consistent include order:

1. Corresponding header
2. Project headers
3. Third-party headers
4. Standard library headers

Separate groups with blank lines.

Example:

```cpp
#include "User.h"

#include "Database.h"

#include <third_party/library.h>

#include <string>
#include <vector>
```

---

# 13. Comments

Code should explain **what** it does through its structure and naming.

Comments should explain **why** something is done when the reason is not obvious.

Avoid comments that merely restate the code.

Bad:

```cpp
// Increment counter
++counter;
```

Good:

```cpp
// The retry counter is incremented before the request because the first
// attempt is counted as retry zero.
++retryCount;
```

Do not leave commented-out code in the codebase.

---

# 14. Magic Numbers and Strings

Avoid unexplained magic numbers and strings.

Prefer:

```cpp
constexpr auto maximumRetries = 3;

if (maximumRetries <= retryCount)
    return;
```

over:

```cpp
if (3 <= retryCount)
    return;
```

unless the value is obvious and local to the expression.

---

# 15. Boolean Expressions

Do not compare booleans unnecessarily.

Prefer:

```cpp
if (isValid)
    return;
```

over:

```cpp
if (isValid == true)
    return;
```

For negation:

```cpp
if (!isValid)
    return;
```

---

# 16. Null Checks

Use `nullptr`.

Never use `NULL` or `0` for null pointers.

---

# 17. Explicit Conversions

Avoid C-style casts.

Do not use:

```cpp
int value = (int)input;
```

Prefer an appropriate C++ cast:

```cpp
int value = static_cast<int>(input);
```

Use the narrowest cast that expresses the intended conversion.

---

# 18. `enum`

Prefer `enum class` over unscoped `enum`.

```cpp
enum class ConnectionState
{
    Disconnected,
    Connecting,
    Connected
};
```

---

# 19. `struct` vs `class`

Use `struct` primarily for passive data types.

Use `class` when the type encapsulates behavior, invariants, or ownership.

---

# 20. Constructors

Construct objects into a valid state.

Avoid constructors that create partially initialized objects.

Prefer member initializer lists.

```cpp
User::User(std::string name)
    : name(std::move(name))
{
}
```

---

# 21. Copy and Move Semantics

Define copy and move operations only when the type's semantics require them.

Prefer the Rule of Zero.

Avoid manually implementing:

* Destructor
* Copy constructor
* Copy assignment operator
* Move constructor
* Move assignment operator

unless necessary.

---

# 22. `override` and `final`

Always use `override` when overriding a virtual method.

```cpp
void process() override;
```

Use `final` when further overriding or inheritance must explicitly be prevented.

---

# 23. Performance

Do not optimize without evidence.

Prefer clear code first.

When performance is important:

1. Measure.
2. Identify the bottleneck.
3. Optimize the bottleneck.
4. Measure again.
5. Document non-obvious optimizations.

Do not sacrifice readability for hypothetical performance improvements.

---

# 24. Testing

Every non-trivial behavior should be testable.

Tests should:

* Test one logical behavior.
* Have descriptive names.
* Be deterministic.
* Avoid unnecessary dependencies.
* Clearly distinguish setup, action, and verification.

---

# 25. File and Namespace Structure

Use namespaces to prevent naming collisions.

Do not use `using namespace` in header files.

Prefer explicit namespace qualification where it improves clarity.

---

# 26. Use of AI

AI may be used to optimize the execution time, correctness, and readability of small code segments, such as individual methods. 

Under no circumstances should AI make decisions regarding architecture, dependencies, or new features. 

AI is a tool for optimizing code quality, not an independent developer or software architect.

---

# 27. AI Coding Rules

Code generated or modified by AI must follow all rules in this document.

AI-generated code must not introduce:

* New coding conventions
* Unrequested abstractions
* Unnecessary dependencies
* Unexplained design patterns
* Global mutable state
* Dead code
* Commented-out code

Before producing code, an AI assistant should verify:

* Every method has exactly one logical responsibility.
* No method exceeds 20 lines.
* No line exceeds 100 characters.
* Naming follows this guide.
* Existing project conventions are preserved.
* No unrelated code was modified.

When modifying existing code, preserve the existing architecture unless the task
explicitly requests architectural changes.

AI assistants must not refactor unrelated code merely because it could be improved.

---

# 28. AI Decision Rules

When multiple valid implementations exist, prefer the implementation that:

1. Is simplest.
2. Has the fewest dependencies.
3. Has the smallest public API.
4. Minimizes mutable state.
5. Is easiest for another developer to understand.

Do not choose a more sophisticated solution unless there is a concrete reason.

If requirements are ambiguous, do not invent business logic.

Use the existing codebase, naming, architecture, and documented behavior as the
primary source of truth.

---

# 29. Change Scope

Changes should be minimal and focused.

A change should modify only what is necessary to fulfill the task.

Do not combine unrelated:

* Refactorings
* Formatting changes
* Renamings
* Dependency updates
* Architecture changes

with a functional change.

If an additional change is necessary for correctness, make the dependency explicit.

---

# 30. Definition of Done

Before considering a change complete, verify:

* [ ] Code compiles.
* [ ] Relevant tests pass.
* [ ] New behavior is covered by tests where appropriate.
* [ ] Methods have one logical responsibility.
* [ ] No method exceeds 20 lines.
* [ ] No line exceeds 100 characters.
* [ ] Naming follows this guide.
* [ ] No unnecessary dependencies were introduced.
* [ ] No unrelated files were changed.
* [ ] No dead or commented-out code remains.
* [ ] The implementation is understandable without AI-specific context.