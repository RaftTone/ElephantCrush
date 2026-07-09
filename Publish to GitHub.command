#!/bin/bash
# One-shot: create the GitHub repo and push ElephantCrush to it.
# First time only you need the GitHub CLI, signed in as you:
#     brew install gh   &&   gh auth login      (choose GitHub.com -> HTTPS -> login in browser)
# Then just double-click this file.
set -e
DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"; cd "$DIR"
REPO_NAME="ElephantCrush"

command -v git >/dev/null 2>&1 || { echo "git not found — install Xcode command line tools."; read -r -p "Return..." _; exit 1; }
if ! command -v gh >/dev/null 2>&1; then
    echo "GitHub CLI (gh) not found. Install once with:  brew install gh"
    echo "Then sign in once with:  gh auth login"
    read -r -p "Press Return to close..." _; exit 1
fi
if ! gh auth status >/dev/null 2>&1; then
    echo "You're not signed in to GitHub CLI. Run once:  gh auth login"
    read -r -p "Press Return to close..." _; exit 1
fi

# init repo if needed
if [ ! -d .git ]; then git init -q; fi
git add -A
git commit -q -m "ElephantCrush 1.0.0 — native Apple Silicon distortion plugin" || echo "(nothing new to commit)"
git branch -M main

if git remote get-url origin >/dev/null 2>&1; then
    echo "Remote already set — pushing update..."
    git push -u origin main
else
    echo "Creating public repo '$REPO_NAME' on your account and pushing..."
    gh repo create "$REPO_NAME" --public --source=. --remote=origin --push \
        --description "Free native Apple-Silicon distortion plugin (AU/VST3) — sounds like Camel Audio's CamelCrusher, rebuilt for arm64. Not affiliated."
fi

URL="$(gh repo view --json url -q .url 2>/dev/null || echo '')"
echo; echo "Done. Your repo:"; echo "  ${URL:-https://github.com/<you>/$REPO_NAME}"
echo "Tip: on the repo page, add Topics (see GITHUB_LISTING.md) and a Release with the Dist/ bundles."
read -r -p "Press Return to close..." _
