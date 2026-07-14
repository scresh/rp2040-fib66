# rp2040-fib66

Demonstration of how reduced loop overhead and loop unrolling affect the ARM assembly generated for
a Fibonacci calculation on a Raspberry Pi Pico W. Each of the three implementations calculates
`F(66) = 27777890035288` on the RP2040's Cortex-M0+ processor.

Successive optimization stages are captured in `main_v1.c`, `main_v2.c`, and `main_v3.c`. The
current v3 implementation is contained in `main.c` and built by CMake.

## Results at a glance

The table compares the simplified cycle model for the calculation loop. It does not represent the
total execution cost of the program.

| Version | Optimization               | Loop iterations | Cycles per iteration | Modelled loop cycles | Speedup |
|---------|----------------------------|-----------------|----------------------|----------------------|---------|
| v1      | Baseline                   | 64              | 13                   | 832                  | 1.00x   |
| v2      | Temporary variable removal | 32              | 8                    | 256                  | 3.25x   |
| v3      | Eightfold loop unrolling   | 4               | 36                   | 144                  | 5.78x   |

The calculation is initialized with `a = 1` and `b = 1`; reaching Fibonacci number 66 therefore
requires 64 remaining steps. The same model is applied to each version:

```text
modelled loop cycles = loop iterations * cycles per iteration
```

## Version 1: Baseline

One Fibonacci step is performed per loop iteration, with a temporary variable used to swap the two
values.

| Assembly instruction | Arguments     | Cycles |
|----------------------|---------------|--------|
| `adds`               | r0, r0, r2    | 1      |
| `adcs`               | r1, r3        | 1      |
| `subs`               | r6, #1        | 1      |
| `movs`               | r4, r0        | 1      |
| `movs`               | r5, r1        | 1      |
| `movs`               | r0, r2        | 1      |
| `movs`               | r1, r3        | 1      |
| `cmp`                | r6, #0        | 1      |
| `beq.n`              | `<main+0x44>` | 1      |
| `movs`               | r2, r4        | 1      |
| `movs`               | r3, r5        | 1      |
| `b.n`                | `<main+0x2c>` | 2      |
| **Total**            |               | **13** |

With one step per iteration, 64 iterations are required: `64 * 13 = 832` modelled loop cycles.

## Version 2: Temporary variable removal

The temporary variable is replaced by two additions, allowing two Fibonacci steps to be performed
per iteration:

```c
b += a;
a += b;
```

| Assembly instruction | Arguments     | Cycles |
|----------------------|---------------|--------|
| `adds`               | r2, r2, r4    | 1      |
| `adcs`               | r3, r5        | 1      |
| `subs`               | r6, #1        | 1      |
| `adds`               | r4, r4, r2    | 1      |
| `adcs`               | r5, r3        | 1      |
| `cmp`                | r6, #0        | 1      |
| `bne.n`              | `<main+0x2c>` | 2      |
| **Total**            |               | **8**  |

With two steps per iteration, the loop count is halved: `32 * 8 = 256` modelled loop cycles.

## Version 3: Eightfold loop unrolling

GCC is instructed to emit the two-step block eight times per outer-loop iteration. Consequently, 16
Fibonacci steps are performed in each outer iteration.

| Assembly work                      | Instructions                   | Repetitions | Cycles per repetition | Total cycles |
|------------------------------------|--------------------------------|-------------|-----------------------|--------------|
| Two Fibonacci steps                | `adds`, `adcs`, `adds`, `adcs` | 8           | 4                     | 32           |
| Decrement loop counter             | `subs`                         | 1           | 1                     | 1            |
| Compare loop counter               | `cmp`                          | 1           | 1                     | 1            |
| Branch to next iteration           | `bne.n`                        | 1           | 2                     | 2            |
| **Total per outer-loop iteration** |                                |             |                       | **36**       |

With 16 steps per outer iteration, the loop count is reduced to four: `4 * 36 = 144` modelled loop
cycles.

## Measurement notes

At the default 125 MHz clock, one cycle takes approximately 8 ns. The cycle model therefore
corresponds to 6.656 us for v1, 2.048 us for v2, and 1.152 us for v3. Measurements made with
`time_us_64()` produced 7 us, 2 us, and 1 us respectively.

These values are illustrative rather than precise benchmarks. The timer has microsecond resolution,
while the model simplifies branch costs and excludes measurement overhead, instruction fetch
effects, and code outside the calculation loop. Generated assembly can also change with the compiler
and optimization settings; the inspected build uses Release mode with `-O3`.
