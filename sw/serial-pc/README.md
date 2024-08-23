# Serial-PC

To verify the score on PCB badge, connect badge to PC by serial and run the command:

```
cargo run -- --dump

# help
cargo run -- --help
```

To connect to a specific serial device, use `--dev` command line argument.

To install `cargo` and relevant rust tools, follow the [Rust website](https://www.rust-lang.org/tools/install).

## Hardware Setup

![badge-serial](https://github.com/user-attachments/assets/3e51b8a7-13aa-42c7-8a3f-4fe058f338c0)

## client

If you run `cargo run` twice, the second instance will become client mode, and you can send data to the badge.

```
cargo run -- --send-game-data --col 0 --data-int 1

# help
cargo run -- --help
```