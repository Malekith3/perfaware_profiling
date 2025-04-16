# Haversine Profiler Playground (Casey Muratori Performance-Aware Part 2)

This project is a hands-on continuation of the [**Performance-Aware Programming**](https://www.computerenhance.com/p/table-of-contents) 
course by Casey Muratori (Part 2 ).  
The primary goal is to **learn how to profile code properly** and gain a deeper understanding of what profiling is and how to use it effectively in real-world scenarios.

## 🧠 What’s the Idea?

- Create a small, self-contained project to **generate, parse, and compute** distances on the sphere using the **Haversine formula**.
- Add profiling to identify bottlenecks and get practical experience with performance optimization.
- Prototype and iterate with a mindset of learning, not building production-quality software.

---

##  Project Structure

### 1. `HaversineCoordGenerator`

A tool to generate random coordinate points on a sphere.

**Features:**
- Supports **uniform** distribution and **clustered** distributions.
- Outputs:
    - `coordinates.json`: Coordinates in a custom JSON format.
    - `distance_answers.f64`: Precomputed Haversine distances stored as raw binary (f64 array). To use for a reference test

---

### 2. `JsonParser`

A **custom, partial JSON parser** written from scratch.

- Tailored to parse only the specific format produced by the generator.
- **Not recursive** and **not compliant with the full JSON spec** — this is intentional.
- Goal: eventually evolve this into a low-level, hand-tuned parser after studying formal languages, tokenization, and lexing.
- For now, it's a "weird partial implementation" that works just enough.

> ⚠️ Warning: It's janky by design. Do not use in production.

---

### 3. `HaversineCLIApp`

A CLI tool to:
- Read the `*.json` and `*.f64` files.
- Compute distances using the Haversine formula.
- Compare with precomputed values.
- **Profile the runtime to find performance bottlenecks**.

**Current bottleneck:** unsurprisingly, the custom JSON parser is the slowest part.  
(Shocking, I know.)

---

##  Why This Project?

- To **practice profiling** and **performance tuning** in a controlled, meaningful setting.
- To explore real trade-offs between clarity, correctness, and speed.

---

##  Future Plans

- Replace the JSON parser with a fully iterative, non-recursive lexer/parser.
- Apply knowledge from formal language theory to make a tiny, optimized parsing engine.

---

##  How to Run

> Coming soon – build/run instructions and profiling commands will be added once stabilized.

---

## Author Notes

> This is a **learning sandbox** — not a polished library.  
It’s a place to try, fail, improve, and **understand performance through doing** and also how **proper parsing done**.
