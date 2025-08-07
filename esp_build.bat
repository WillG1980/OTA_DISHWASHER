@echo off
git add .
git diff --cached --quiet || (
    git commit -m "Auto-commit on build: %DATE% %TIME%" 
    FOR /F %%i IN ('git rev-list --count HEAD') DO git tag -a build-%%i -m "Build tag"
)
idf.py build