VERSION := $(shell cat VERSION)
PIO ?= pio
ENV ?= d1_mini
DIST_DIR ?= dist
BINARY := $(DIST_DIR)/brauknecht-v$(VERSION)-$(ENV).bin

.PHONY: all build binary clean version bump-patch bump-minor bump-major tag-version release-patch release-minor release-major test

all: build

build:
	$(PIO) run -e $(ENV)

binary: build
	@mkdir -p "$(DIST_DIR)"
	cp ".pio/build/$(ENV)/firmware.bin" "$(BINARY)"
	@printf "%s\n" "$(BINARY)"

clean:
	$(PIO) run -e $(ENV) -t clean

test:
	$(PIO) test -e native
	sh scripts/test_version_scripts.sh

version:
	@printf "v%s\n" "$$(cat VERSION)"

bump-patch:
	./scripts/bump-version.sh patch

bump-minor:
	./scripts/bump-version.sh minor

bump-major:
	./scripts/bump-version.sh major

tag-version:
	./scripts/tag-version.sh

release-patch:
	./scripts/bump-version.sh patch
	./scripts/tag-version.sh

release-minor:
	./scripts/bump-version.sh minor
	./scripts/tag-version.sh

release-major:
	./scripts/bump-version.sh major
	./scripts/tag-version.sh
