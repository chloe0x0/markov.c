<h1 align="center">
markov.c
</h1>

A simple markov chain utility for text generation in C.

Probably a nightmare of a codebase but most of this was written while high so who cares.

## Building

```console
make markov
```

## Usage

```console
> markov.exe is-char[bool] N[int] iters(int) init_state[string] [text files]
```

is_char determines whether or not each state in the markov chain will be 'N' characters or 'N' words. Iters is the number of states to sample/ generate. Iters=25 will generate 25 new states and appened them to a string, for example. init_state is the initial string. If init_state is {RAND}, a random initial state will be selected. If {START} is chosen, the start of the training data will be used.

example usage, 

```
./markov 0 2 50 {RAND} data/shakespeare.txt
Than most have of HUBERT. I am much deceiv'd, cuckolds ere now; And many unfrequented plots there are few die well that what you MARCUS. Stand by me, Do no more offices of life By some vile forfeit of untimely But he would bite none; just as I wooed for thee to my strong enforcement be; In the corrupted currents of this deeds; or, by the hand of she here- what's her Since she respects my mistress' love so much. AGAMEMNON. This Troyan scorns us, or it mars us; think on that, And fix most firm thy resolution. RODERIGO. Be 
```