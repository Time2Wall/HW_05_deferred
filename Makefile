# HW_05_deferred_exec_mech/Makefile
# Build three example modules:
#   ex_softirq.ko   — demonstrates SoftIRQ context via irq_work
#   ex_tasklets.ko  — demonstrates tasklets (normal & high-priority)
#   ex_workqueue.ko — demonstrates system and dedicated workqueues

# Kernel build integration
KDIR ?= /lib/modules/$(shell uname -r)/build
PWD  := $(shell pwd)

obj-m += ex_softirq.o
obj-m += ex_tasklets.o
obj-m += ex_workqueue.o

# Default target: build all modules
.PHONY: all
all: modules

.PHONY: modules
modules:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Clean generated files
.PHONY: clean
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

# Format source files with clang-format if available
.PHONY: format
format:
	@if command -v clang-format >/dev/null 2>&1; then \
	  echo "[format] Running clang-format on *.c"; \
	  clang-format -i $(wildcard *.c); \
	else \
	  echo "[format] clang-format not found; skipping."; \
	fi

# Run kernel's checkpatch.pl if available
.PHONY: check
check:
	@CHECKPATCH="$(KDIR)/scripts/checkpatch.pl"; \
	if [ -x $$CHECKPATCH ]; then \
	  for f in $(wildcard *.c); do \
	    echo "== Checking $$f =="; \
	    $$CHECKPATCH --no-tree --file --strict $$f || true; \
	  done; \
	else \
	  echo "[check] $(KDIR)/scripts/checkpatch.pl not found; skipping."; \
	fi

# Alias: the assignment asks for 'make'
.PHONY: make
make: modules
