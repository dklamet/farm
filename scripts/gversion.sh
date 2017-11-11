#!/bin/bash

isdirty()
{
    if [ -n "`git diff-index --name-only HEAD 2>&1`" ] ; then
        printf '%s' -dirty
    fi
}

#Print commit or tag. Append -dirty if any tracked files have been modified
    head=`git rev-parse --verify --short HEAD 2>/dev/null`
    branch=`git symbolic-ref --short -q HEAD 2>/dev/null `

    tag=`git describe --exact-match 2>/dev/null`

    if [ -n "$branch" ]; then
        echo -n "$branch: "
    fi
    if [ -n "$tag" ]; then
        echo -n "$tag"
        isdirty
    elif [ -n "$head" ]; then
        echo -n $head
        isdirty
    else
        echo -n "Unversioned"
    fi

    echo ""
date
hostname


