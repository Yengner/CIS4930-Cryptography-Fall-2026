# GitHub Setup

Recommended repository name:

`CIS4930-Cryptography-Fall-2026`

Keep the repository **private** while the course is active.

## Push this prepared repository

After creating an empty private repository on GitHub with the name above, open Terminal in this folder and run:

```bash
git remote add origin https://github.com/Yengner/CIS4930-Cryptography-Fall-2026.git
git push -u origin main
```

Then invite your teammate from the repository's **Settings -> Collaborators** page.

## Normal workflow with your teammate

Before working:

```bash
git pull
```

After making changes:

```bash
git add .
git commit -m "Describe what you changed"
git push
```

For each new homework, create a new folder such as `HW2`, `HW3`, and so on.
