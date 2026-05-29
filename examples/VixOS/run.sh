#!/usr/bin/env bash
set -euo pipefail
ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$ROOT_DIR"
TARGET="i386-unknown-none"
KERNEL="kernel.vix"
ASM="asm_stubs.s"
OBJ="kernel.o"
ASM_OBJ="asm_stubs.o"
ELF="kernel.elf"
ISO="vixos.iso"

echo "[1/5] Compiling kernel..."
vixc "$KERNEL" -obj "$OBJ" --target="$TARGET" -opt=l2

echo "[2/5] Assembling stubs..."
as --32 "$ASM" -o "$ASM_OBJ"

echo "[3/5] Linking..."
ld.lld -m elf_i386 -T linker.ld "$OBJ" "$ASM_OBJ" -o "$ELF"

echo "[4/5] Building ISO..."
cp "$ELF" iso/boot/kernel.elf
grub-mkrescue -o "$ISO" iso 2>/dev/null

echo "[5/5] Launching QEMU..."
echo "  Use Ctrl+A, X to exit QEMU"
qemu-system-i386 -cdrom "$ISO" -display curses || true

rm -f "$OBJ" "$ASM_OBJ" "$ELF" kernel.ll
