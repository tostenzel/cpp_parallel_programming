# Parallel Programming - Spring/Summer Term 2022

* Please use one folder of the name `exXX` for each of your solutions.
* Please use one folder for each of your solutions.
* Please put in your email addresses into the `email.txt` at the repository's root with one email address per line.
* Don't put anything else into the repository's root.
* Your solution will simply be the state of your repository of the `master` branch just at the deadline.

In later phases of the lecture, I will add templates for the upcoming programming exercises that you have to merge into your current repository.
With the following setup you will add another remote `template` for this template repository:
```
git remote add template ${group}@728119b8-1afd-42d0-9b7e-5f8385271a39.ma.bw-cloud-instance.org:/home/ubuntu/template.git
```
These commands lets you merge new developments in `template` into you current branch:
```
git fetch --all
git merge template/master
```
