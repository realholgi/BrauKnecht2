#!/usr/bin/env sh
set -eu

version_file="${VERSION_FILE:-VERSION}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    printf 'not inside a git work tree\n' >&2
    exit 1
fi

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
case "$1$2$3" in
    *[!0-9]*)
        printf 'invalid semantic version: %s\n' "$version" >&2
        exit 1
        ;;
esac

tag="v${version}"
if git rev-parse -q --verify "refs/tags/${tag}" >/dev/null; then
    printf 'tag already exists: %s\n' "$tag" >&2
    exit 1
fi

if ! git diff --quiet -- "$version_file" || ! git diff --cached --quiet -- "$version_file" || ! git ls-files --error-unmatch "$version_file" >/dev/null 2>&1; then
    git add "$version_file"
    git commit -m "Release ${tag}" -- "$version_file"
fi

git tag -a "$tag" -m "$tag"
printf '%s\n' "$tag"
