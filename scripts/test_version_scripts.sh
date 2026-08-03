#!/usr/bin/env sh
set -eu

tmpdir="$(mktemp -d)"
trap 'rm -rf "$tmpdir"' EXIT

cp scripts/bump-version.sh "$tmpdir/bump-version.sh"
cp scripts/version-string.sh "$tmpdir/version-string.sh"

run_bump() {
    kind="$1"
    start="$2"
    expected="$3"
    printf '%s\n' "$start" > "$tmpdir/VERSION"
    (cd "$tmpdir" && ./bump-version.sh "$kind" >/dev/null)
    actual="$(cat "$tmpdir/VERSION")"
    if [ "$actual" != "$expected" ]; then
        printf 'expected %s bump from %s to produce %s, got %s\n' "$kind" "$start" "$expected" "$actual" >&2
        exit 1
    fi
}

run_bump patch 1.2.3 1.2.4
run_bump minor 1.2.3 1.3.0
run_bump major 1.2.3 2.0.0

printf 'bad.version\n' > "$tmpdir/VERSION"
if (cd "$tmpdir" && ./bump-version.sh patch >/dev/null 2>&1); then
    printf 'invalid VERSION unexpectedly succeeded\n' >&2
    exit 1
fi

assert_version_string() {
    repo="$tmpdir/repo-$1"
    expected="$2"
    mkdir "$repo"
    cp scripts/version-string.sh "$repo/version-string.sh"
    (
        cd "$repo"
        git init -q
        git config user.email test@example.invalid
        git config user.name "Version Test"
        git config commit.gpgsign false
        printf '1.2.3\n' > VERSION
        git add VERSION
        git commit -q -m initial
        case "$1" in
            release)
                git tag v1.2.3
                ;;
            mismatch)
                git tag v1.2.2
                ;;
            dirty)
                printf 'change\n' > extra.txt
                ;;
        esac
        actual="$(./version-string.sh)"
        if [ "$actual" != "$expected" ]; then
            printf 'expected %s version string %s, got %s\n' "$1" "$expected" "$actual" >&2
            exit 1
        fi
    )
}

assert_version_string release 1.2.3
assert_version_string mismatch 1.2.3-dev
assert_version_string dirty 1.2.3-dev
