/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_grid.h
        external function/variables/defines for grid widget
        this file is part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/

#ifndef _GUI_GRID_H_
#define _GUI_GRID_H_

typedef struct grid_column
{
    int (*getSize)      (struct grid_column *, int *, int *);
    int (*setString)    (struct grid_column *, char *);
    char *(*getString)  (struct grid_column *);
    int (*setNextColumn)(struct grid_column *, struct grid_column *);
    int (*setPrevColumn)(struct grid_column *, struct grid_column *);
    struct grid_column *(*getNextColumn) (struct grid_column *);
    struct grid_column *(*getPrevColumn) (struct grid_column *);

#ifdef _ENABLE_GRID_COLUMN_INTERNAL_INTERFACE

#endif  /* _ENABLE_GRID_COLUMN_INTERNAL_INTERFACE_ */
}
grid_column_t;


typedef struct grid_row
{
    int (*setRowIndex)  (struct grid_row *, int);
    int (*getRowIndex)  (struct grid_row *);
    int (*getNumColumns)(struct grid_row *);
    int (*setPrevRow)  (struct grid_row *, struct grid_row *);
    int (*setNextRow)  (struct grid_row *, struct grid_row *);
    struct grid_row *(*getPrevRow)   (struct grid_row *);
    struct grid_row *(*getNextRow)   (struct grid_row *);
    struct grid_column *(*addColumn)    (struct grid_row *, char *, int, int);
    struct grid_column *(*setPrevColumn)(struct grid_row *);
    struct grid_column *(*setNextColumn)(struct grid_row *);
    struct grid_column *(*setHeadColumn)(struct grid_row *);
    struct grid_column *(*setTailColumn)(struct grid_row *);
    struct grid_column *(*getCurrColumn)(struct grid_row *);

#ifdef _ENABLE_GRID_ROW_INTERNAL_INTERFACE_

#endif /* _ENABLE_GRID_ROW_INTERNAL_INTERFACE_ */
}
grid_row_t;

typedef struct gui_grid_interface
{
    int (*setPosition)       (struct gui_grid_interface *, rect_t *);
    int (*getPosition)       (struct gui_grid_interface *, rect_t *);
    int (*getNumRows)        (struct gui_grid_interface *);
    int (*removeRow)         (struct gui_grid_interface *);
    int (*updateRowIndex)    (struct gui_grid_interface *);
    int (*addHeaderColumn)   (struct gui_grid_interface *, char *, int, int);
    int (*getNumColumnHeader)(struct gui_grid_interface *);

    struct grid_row *(*addRow)         (struct gui_grid_interface *);
    struct grid_row *(*getRow)         (struct gui_grid_interface *, int index);
    struct grid_row *(*getHeadRow)     (struct gui_grid_interface *);
    struct grid_row *(*getTailRow)     (struct gui_grid_interface *);
    struct grid_row *(*getCurrRow)     (struct gui_grid_interface *);
    struct grid_row *(*setPrevRow)     (struct gui_grid_interface *);
    struct grid_row *(*setNextRow)     (struct gui_grid_interface *);
    struct grid_row *(*getColumnHeader)(struct gui_grid_interface *);

#ifdef _ENABLE_GRID_INTERNAL_INTERFACE_

#endif /* _ENABLE_GRID_INTERNAL_INTERFACE_ */
}
grid_t;


/* external functions for create/destroy grid widget */
extern struct gui_grid_interface *gui_grid_create(void);
extern int gui_grid_destroy(struct gui_grid_interface *grid);

#endif /* _GUI_GRID_H_ */
