/***********************************************************************
 *                                                                      *
 *               This software is part of the ast package               *
 *          Copyright (c) 1985-2011 AT&T Intellectual Property          *
 *                      and is licensed under the                       *
 *                 Eclipse Public License, Version 1.0                  *
 *                    by AT&T Intellectual Property                     *
 *                                                                      *
 *                A copy of the License is available at                 *
 *          http://www.eclipse.org/org/documents/epl-v10.html           *
 *         (with md5 checksum b35adb5213ca9657e911e9befb180842)         *
 *                                                                      *
 *              Information and Software Systems Research               *
 *                            AT&T Research                             *
 *                           Florham Park NJ                            *
 *                                                                      *
 *               Glenn Fowler <glenn.s.fowler@gmail.com>                *
 *                    David Korn <dgkorn@gmail.com>                     *
 *                     Phong Vo <phongvo@gmail.com>                     *
 *                                                                      *
 ***********************************************************************/
//
// Glenn Fowler
// AT&T Research
//
// Return 1 if path exisis
// Maintains a cache to minimize stat(2) calls
// Path is modified in-place but restored on return
// Path components checked in pairs to cut stat()'s
// in half by checking ENOTDIR vs. ENOENT
// Case ignorance infection unavoidable here
//
#include "config_ast.h"  // IWYU pragma: keep

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ast.h"

// This tree caches name and modes of files and directories
typedef struct Tree_s {
    // `next` points to next subdirectory of parent directory
    struct Tree_s *next;
    // If current node is a directory, `name` and `modes` of it's subdirectories and files are
    // cached under this tree
    struct Tree_s *tree;
    // Modes can be PATH_READ, PATH_WRITE and PATH_EXECUTE
    int mode;
    char name[1];
} Tree_t;

int pathexists(char *path, int mode) {
    // Path is splitted by '/''s and each component is processed seaprately
    // This variable points to current component of path that is being processed
    char *current_path_component;

    // Next path component
    char *next_path_component;

    // Points to next slash in path
    char *next_slash;

    // Points to parent directory
    Tree_t *parent_tree;
    Tree_t *current_tree;

    int c;
    int cc;
    int x;
    struct stat statbuf;

    static Tree_t tree;

    current_tree = &tree;
    // If path starts with `/`, initialize next slash character after first character
    next_slash = (c = *path) == '/' ? path + 1 : path;
    while (c) {
        parent_tree = current_tree;
        // Try to search for next slash character
        for (current_path_component = next_slash; *next_slash && *next_slash != '/'; next_slash++) {
            ;
        }

        // Save value pointed by `next_slash` variable
        c = *next_slash;
        // and put a null character there to mark end of path, so `foo/bar/baz` becomes `foo`
        *next_slash = 0;
        for (current_tree = parent_tree->tree;
             current_tree && strcmp(current_path_component, current_tree->name);
             current_tree = current_tree->next) {
            ;
        }

        // Path with name `current_path_name` does not exist in tree, time to create a new node.
        if (!current_tree) {
            current_tree = calloc(1, sizeof(Tree_t) + strlen(current_path_component));
            if (!current_tree) {
                *next_slash = c;
                return 0;
            }
            strcpy(current_tree->name, current_path_component);
            current_tree->next = parent_tree->tree;
            parent_tree->tree = current_tree;

            // If `c` is set, we are not at end of path
            if (c) {
                *next_slash = c;
                for (current_path_component = next_path_component = next_slash + 1;
                     *next_path_component && *next_path_component != '/'; next_path_component++) {
                    ;
                }
                cc = *next_path_component;
                *next_path_component = 0;
            } else {
                next_path_component = 0;
            }
            x = stat(path, &statbuf);

            // Create a new node for subdirectory (or file)
            if (next_path_component) {
                Tree_t *new_tree;
                next_slash = next_path_component;
                c = cc;
                if (!x || errno == ENOENT) current_tree->mode = PATH_READ | PATH_EXECUTE;
                new_tree = calloc(1, sizeof(Tree_t) + strlen(current_path_component));
                if (!new_tree) {
                    *next_slash = c;
                    return 0;
                }
                strcpy(new_tree->name, current_path_component);
                new_tree->next = current_tree->tree;
                current_tree->tree = new_tree;
                // Set new node as current node
                current_tree = new_tree;
            }
            if (x) {
                *next_slash = c;
                return 0;
            }
            if (statbuf.st_mode & (S_IRUSR | S_IRGRP | S_IROTH)) current_tree->mode |= PATH_READ;
            if (statbuf.st_mode & (S_IWUSR | S_IWGRP | S_IWOTH)) current_tree->mode |= PATH_WRITE;
            if (statbuf.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) current_tree->mode |= PATH_EXECUTE;
            if (!S_ISDIR(statbuf.st_mode)) current_tree->mode |= PATH_REGULAR;
        }

        // Restore `/` character
        *next_slash++ = c;
        if (!current_tree->mode || (c && (current_tree->mode & PATH_REGULAR))) return 0;
    }
    mode &= (PATH_READ | PATH_WRITE | PATH_EXECUTE | PATH_REGULAR);
    return (current_tree->mode & mode) == mode;
}
