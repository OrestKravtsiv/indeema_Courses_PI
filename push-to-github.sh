#!/bin/bash
# Helper script to push your code to GitHub

echo "==============================================="
echo "GitHub Repository Setup for CoreApp"
echo "==============================================="

# Check if git is initialized
if [ ! -d ".git" ]; then
    echo "Initializing git repository..."
    git init
    git branch -M main
fi

# Get GitHub details
echo ""
echo "Enter your GitHub username:"
read -r github_user

echo "Enter your repository name (e.g., coreapp-embedded):"
read -r github_repo

# Add all files
echo ""
echo "Adding files to git..."
git add .

# Commit
echo "Enter commit message [default: 'Initial commit: CoreApp for embedded Linux']:"
read -r commit_msg
if [ -z "$commit_msg" ]; then
    commit_msg="Initial commit: CoreApp for embedded Linux"
fi

git commit -m "$commit_msg"

# Add remote
echo ""
echo "Adding remote repository..."
git remote remove origin 2>/dev/null || true
git remote add origin "https://github.com/${github_user}/${github_repo}.git"

# Push
echo ""
echo "Pushing to GitHub..."
echo "You may be prompted for your GitHub credentials."
git push -u origin main

echo ""
echo "==============================================="
echo "Repository pushed successfully!"
echo "URL: https://github.com/${github_user}/${github_repo}"
echo ""
echo "Next steps:"
echo "1. Update meta-coreapp/recipes-app/coreapp/coreapp_git.bb"
echo "   - Replace YOUR_USERNAME with: ${github_user}"
echo "   - Replace YOUR_REPO with: ${github_repo}"
echo "2. Run the setup-integration.sh script in meta-coreapp/"
echo "==============================================="
