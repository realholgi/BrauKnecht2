#!/usr/bin/env sh
set -eu

kind="${1:-}"
version_file="${VERSION_FILE:-VERSION}"

case "$kind" in
    patch|minor|major) ;;
    *)
        printf 'usage: %s patch|minor|major\n' "$0" >&2
        exit 2
        ;;
esac

if [ ! -f "$version_file" ]; then
    printf '%s not found\n' "$version_file" >&2
    exit 1
fi

version="$(tr -d '[:space:]' < "$version_file")"
case "$version" in
    *[!0-9.]*|*.*.*.*|.*|*.)
        printf 'invalid semantic version: %s\n' "$version" >&2
        exit 1
        ;;
esac

old_ifs="$IFS"
IFS=.
set -- $version
IFS="$old_ifs"

if [ "$#" -ne 3 ] || [ -z "${1:-}" ] || [ -z "${2:-}" ] || [ -z "${3:-}" ]; then
    printf 'invalid semantic version: %s\n' "$version" >&2
    exit 1
fi

major="$1"
minor="$2"
patch="$3"

case "$major$minor$patch" in
    *[!0-9]*)
        printf 'invalid semantic version: %s\n' "$version" >&2
        exit 1
        ;;
esac

case "$kind" in
    patch)
        patch=$((patch + 1))
        ;;
    minor)
        minor=$((minor + 1))
        patch=0
        ;;
    major)
        major=$((major + 1))
        minor=0
        patch=0
        ;;
esac

new_version="${major}.${minor}.${patch}"
printf '%s\n' "$new_version" > "$version_file"
printf 'v%s\n' "$new_version"
