# Dynamic Array Implementation

My implemented container, ```DynamicArray```, tries to mimic ```std::vector``` as a container that allows the following features:

1) Insertion of elements
2) Deletion of elements
3) Lookup of elements

on a high-level.

---

## Challenges

In more technical detail, the container tries to be robust, efficient and idiomatic in the way it is written. This involves writing code that can handle:

1) Various Datatypes
- Primitive Types like ```int```, ```bool```, ```char```
- Standard Types like ```string```, ```unique_ptr``` etc
- User Defined Types like ```structs``` and ```classes```

The container needs to also ensure over-aligned types are dealt with accordingly.

2) Various movement semantics
- Move-Only types that can not be copied
- Copy-Only types that can not be moved
- Types that are capable of being moved and copied

Choosing a strategy to execute a function depends heavily on whether the type has certain properties.

3) Various degrees of Exception Safety

In every aforementioned type, we also have to deal with constructors and assignment operations that can throw. The container must implement strategies that can:

- Ensure Strong Exception Safety Guarantee whenever possible
- Degrade to Basic Exception Safety in other cases
- For both types of guarantees, make efficient operations a priority

---

## Limitations

Keeping all of this in mind, it is necessary to define minimal but strict limitations on the nature of type T that the container stores so that it stays robust and safe while also not being too restrictive. In my case, as is the case in std::vector,

- Destructor of type must not throw, otherwise program terminates
- Type must be copy constructible to ensure copy constructor and assignment work for container

These are enforced as static assertions that disallow container creation if these conditions are not met. Apart from this, the container also expects certain behavior from specific types for correct operation:

- For move-only types, moves are expected to be ```noexcept``` as a convention
- Similarly, for copy-only types, copying is expected to be ```noexcept```
