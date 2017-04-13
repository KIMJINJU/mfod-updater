/*
    Copyright (C) 2016 EOSYSTEM CO., LTD. (www.eosystem.com)

    gui_grid.c
        external/internal function implementations of grid widget interface
        this file is  part of amod-mainapp

    written by
        Seung-hwan, Park (shpark2@eosystem.com)
*/


#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "core/logger.h"
#include "gui/gui_grid.h"


/* constant macro defines - grid field, column string length */
#define GRID_COLUMN_MAXSTRLEN   64
#define MAX_GRID_ROWS           128

/* structure declarations -  grid column attributes */
struct grid_column_attribute
{
    /* external interface */
    struct grid_column extif;

    /* internal interface */
    int width;
    int height;
    char string[GRID_COLUMN_MAXSTRLEN];
    struct grid_column *prevColumn;
    struct grid_column *nextColumn;
};

/* structure declarations -  grid row attributes */
struct grid_row_attribute
{
    /* external interface */
    struct grid_row extif;

    /* internal interface */
    int index;
    int numColumns;
    struct grid_column *headColumn;
    struct grid_column *tailColumn;
    struct grid_column *currColumn;
    struct grid_row *prevRow;
    struct grid_row *nextRow;
};

/* structure declarations -  grid widget attributes */
struct gui_grid_attribute
{
    /* external interface */
    struct gui_grid_interface extif;

    /* internal interface */
    int numRows;
    int numHeaderColumn;
    rect_t position;
    struct grid_row *headRow;
    struct grid_row *tailRow;
    struct grid_row *currRow;
    struct grid_row *headerColumn;
};


static int
gui_grid_set_column_string(struct grid_column *col, char *string)
{
    int ret = 0;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
    {
        memset(this->string, 0x00, sizeof(this->string));
        strncpy(this->string, string, GRID_COLUMN_MAXSTRLEN);
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null gird column pointer\n", DBGINFO));

    return ret;
}


static char *
gui_grid_get_column_string(struct grid_column *col)
{
    char *str = NULL;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
        str = &this->string[0];
    else
        TLOGMSG(1, (DBGINFOFMT "null gird column pointer\n", DBGINFO));

    return str;
}


static int
gui_grid_get_column_size(struct grid_column *col, int *w, int *h)
{
    int ret = 0;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
    {
        *w = this->width;
        *h = this->height;
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid column pointer\n", DBGINFO));
    }

    return ret;
}


static struct grid_column *
gui_grid_get_next_column(struct grid_column *col)
{
    struct grid_column *next = NULL;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
        next = this->nextColumn;
    else
        TLOGMSG(1, (DBGINFOFMT "null grid column pointer\n", DBGINFO));

    return next;
}


static struct grid_column *
gui_grid_get_prev_column(struct grid_column *col)
{
    struct grid_column *prev = NULL;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
        prev = this->prevColumn;
    else
        TLOGMSG(1, (DBGINFOFMT "null grid column pointer\n", DBGINFO));

    return prev;
}


static int
gui_grid_set_next_column(struct grid_column *col, struct grid_column *next)
{
    int ret = 0;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
        this->nextColumn = next;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid column pointer\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_set_prev_column(struct grid_column *col, struct grid_column *prev)
{
    int ret = 0;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
        this->prevColumn = prev;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid column pointer\n", DBGINFO));
    }

    return ret;
}


static struct grid_column *
gui_grid_create_column(char *string, int w, int h)
{
    struct grid_column *col = NULL;
    struct grid_column_attribute *this = malloc(sizeof(struct grid_column_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct grid_column_attribute));
        strncpy(this->string, string, GRID_COLUMN_MAXSTRLEN);
        this->width  = w;
        this->height = h;
        col = &(this->extif);
        col->getSize   = gui_grid_get_column_size;
        col->setString = gui_grid_set_column_string;
        col->getString = gui_grid_get_column_string;
        col->getNextColumn = gui_grid_get_next_column;
        col->getPrevColumn = gui_grid_get_prev_column;
        col->setNextColumn = gui_grid_set_next_column;
        col->setPrevColumn = gui_grid_set_prev_column;
        TLOGMSG(0, ("create grid column (width = %d, height = %d, string = %s\n)", w, h, string));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return col;
}


static int
gui_grid_destroy_column(grid_column_t *col)
{
    int ret = 0;
    struct grid_column_attribute *this = (struct grid_column_attribute *) col;

    if (this)
    {
        free(this);
        TLOGMSG(0, ("destroy grid column\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid column pointer\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_set_row_index(struct grid_row *row, int idx)
{
    int ret = 0;
    struct grid_row_attribute *this = (struct grid_row_attribute *)row;

    if (this)
        this->index  = idx;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_get_row_index(struct grid_row *row)
{
    int ret = 0;
    struct grid_row_attribute *this = (struct grid_row_attribute *)row;

    if (this)
        ret = this->index;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_get_ncols(struct grid_row *row)
{
    int ret = 0;
    struct grid_row_attribute *this = (struct grid_row_attribute *)row;

    if (this)
        ret = this->numColumns;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));
    }

    return ret;
}


static struct grid_column *
gui_grid_add_column(struct grid_row *row, char *str, int w, int h)
{
    struct grid_column *col = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *)row;

    if (this)
    {
        col = gui_grid_create_column(str, w, h);

        if (col)
        {
            if (this->numColumns == 0)
            {
                this->headColumn = col;
                this->tailColumn = col;
                this->currColumn = col;
                this->headColumn->setNextColumn(this->headColumn, this->tailColumn);
                this->headColumn->setPrevColumn(this->headColumn, this->tailColumn);
            }
            else
            {
                col->setNextColumn(col, this->headColumn);
                col->setPrevColumn(col, this->tailColumn);
                this->headColumn->setPrevColumn(this->headColumn, col);
                this->tailColumn->setNextColumn(this->tailColumn, col);
                this->tailColumn = col;

                //this->headColumn->setNextColumn(this->headColumn, col);
                //this->headColumn->setPrevColumn(this->headColumn, col);
                //this->headColumn = col;
            }

            this->numColumns++;
        }
        else
            TLOGMSG(1, (DBGINFOFMT "failed to create column\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return col;
}


static struct grid_column *
gui_grid_get_curr_column(struct grid_row *row)
{
    struct grid_column *col = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *)row;

    if (this)
        col = this->currColumn;
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return col;
}


static struct grid_column *
gui_grid_prev_column(struct grid_row *row)
{
    struct grid_column *col = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *)row;

    if (this)
    {
        this->currColumn = this->currColumn->getPrevColumn(this->currColumn);
        col = this->currColumn;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return col;
}


static struct grid_column *
gui_grid_next_column(struct grid_row *row)
{
    struct grid_column *col = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
    {
        this->currColumn = this->currColumn->getNextColumn(this->currColumn);
        col = this->currColumn;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return col;
}


static struct grid_column *
gui_grid_head_column(struct grid_row *row)
{
    struct grid_column *col = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
    {
        this->currColumn = this->headColumn;
        col = this->currColumn;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return col;
}


static struct grid_column *
gui_grid_tail_column(struct grid_row *row)
{
    struct grid_column *col = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
    {
        this->currColumn = this->tailColumn;
        col = this->currColumn;
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return col;
}


static struct grid_row *
gui_grid_get_prev_row(struct grid_row *row)
{
    struct grid_row *prev = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
        prev = this->prevRow;
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return prev;
}


static struct grid_row *
gui_grid_get_next_row(struct grid_row *row)
{
    struct grid_row *next = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
        next = this->nextRow;
    else
        TLOGMSG(1, (DBGINFOFMT "null row pointer\n", DBGINFO));

    return next;
}


static int
gui_grid_set_prev_row(struct grid_row *row, struct grid_row *prev)
{
    int ret = 0;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
        this->prevRow = prev;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT"null row pointer\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_set_next_row(struct grid_row *row, struct grid_row *next)
{
    int ret = 0;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
        this->nextRow = next;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT"null row pointer\n", DBGINFO));
    }

    return ret;
}


static struct grid_row *
gui_grid_create_row()
{
    struct grid_row *row = NULL;
    struct grid_row_attribute *this = malloc(sizeof(struct grid_row_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct grid_row_attribute));
        this->headColumn = NULL;
        this->tailColumn = NULL;
        this->currColumn = NULL;
        this->prevRow = NULL;
        this->nextRow = NULL;

        row = &(this->extif);
        row->setRowIndex   = gui_grid_set_row_index;
        row->getRowIndex   = gui_grid_get_row_index;
        row->getNumColumns = gui_grid_get_ncols;
        row->addColumn     = gui_grid_add_column;
        row->setPrevColumn = gui_grid_prev_column;
        row->setNextColumn = gui_grid_next_column;
        row->setHeadColumn = gui_grid_head_column;
        row->setTailColumn = gui_grid_tail_column;
        row->getCurrColumn = gui_grid_get_curr_column;
        row->setNextRow = gui_grid_set_next_row;
        row->setPrevRow = gui_grid_set_prev_row;
        row->getNextRow = gui_grid_get_next_row;
        row->getPrevRow = gui_grid_get_prev_row;
        TLOGMSG(0, ("create row (0x%p) for grid widget\n", row));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return row;
}


static int
gui_grid_destroy_row(struct grid_row * row)
{
    int ret = 0;
    struct grid_column *col = NULL;
    struct grid_row_attribute *this = (struct grid_row_attribute *) row;

    if (this)
    {
        while (this->numColumns != 0)
        {
            col = this->headColumn;
            this->headColumn = this->headColumn->getNextColumn(this->headColumn);
            gui_grid_destroy_column(col);
            this->numColumns--;
        }

        free(this);
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid row pointer\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_set_position(struct gui_grid_interface *grid, rect_t *pos)
{
    int ret = 0;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *) grid;

    if (this)
    {
        memcpy(&this->position, pos, sizeof(rect_t));
        TLOGMSG(1, ("grid position : x = %d, y = %d, w = %d, h = %d\n", pos->x, pos->y, pos->w, pos->h));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_get_nrows(struct gui_grid_interface *grid)
{
    int ret = 0;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *) grid;

    if (this)
        ret = this->numRows;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_get_position(struct gui_grid_interface *grid, rect_t *pos)
{
    int ret = 0;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *) grid;

    if (this)
        memcpy(pos, &this->position, sizeof(rect_t));
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_remove_row(struct gui_grid_interface *grid)
{
    int ret = 0;
    struct grid_row *row = NULL;
    struct grid_row *temp = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *) grid;

    if (this)
    {
        if (this->numRows > 0)
        {
            row = this->currRow;

            if (this->currRow == this->headRow)
            {
                this->headRow = row->getNextRow(row);
                this->headRow->setPrevRow(this->headRow, this->tailRow);
                this->tailRow->setNextRow(this->tailRow, this->headRow);
                this->currRow = this->headRow;
            }
            else if (this->currRow == this->tailRow)
            {
                this->tailRow = row->getPrevRow(row);
                this->tailRow->setNextRow(this->tailRow, this->headRow);
                this->headRow->setPrevRow(this->headRow, this->tailRow);
                this->currRow = this->tailRow;
            }
            else
            {
                temp = row->getPrevRow(row);
                temp->setNextRow(temp, row->getNextRow(row));
                temp = row->getNextRow(row);
                temp->setPrevRow(temp, row->getPrevRow(row));
                this->currRow = row->getNextRow(row);
            }

            this->numRows--;
            grid->updateRowIndex(grid);
            gui_grid_destroy_row(row);
        }
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "no rows to remove from grid widget\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_update_row_index(struct gui_grid_interface *grid)
{
    int ret = 0;
    struct grid_row *row = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
    {
        row = this->headRow;

        for (int i = 0; i < this->numRows; i++)
        {
            row->setRowIndex(row, i);
            row = row->getNextRow(row);
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_add_header_column(struct gui_grid_interface *grid, char *str, int w, int h)
{
    int ret = 0;
    struct grid_column *col = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
    {
        col = this->headerColumn->addColumn(this->headerColumn, str, w, h);

        if (col)
            TLOGMSG(1, ("add column header (0x%p, str = %s, w = %d, h = %d)\n", col, str, w, h));
        else
        {
            ret = -1;
            TLOGMSG(1, (DBGINFOFMT "failed to clreate column header\n", DBGINFO));
        }
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}


static int
gui_grid_get_number_header_columns(struct gui_grid_interface *grid)
{
    int ret = 0;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
        ret = this->numHeaderColumn;
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}


static struct grid_row *
gui_grid_add_row(struct gui_grid_interface *grid)
{
    struct grid_row *row = NULL;
    struct grid_row *temp = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
    {
        row = gui_grid_create_row();

        if (row)
        {
            if (this->numRows < MAX_GRID_ROWS)
            {
                if (this->numRows == 0)
                {
                    row->setNextRow(row, row);
                    row->setPrevRow(row, row);
                    this->headRow = row;
                    this->currRow = row;
                }
                else
                {
                    row->setNextRow(row, this->headRow);
                    row->setPrevRow(row, this->tailRow);
                    this->headRow->setPrevRow(this->headRow, row);
                    this->tailRow->setNextRow(this->tailRow, row);
                }

                this->tailRow = row;
                this->numRows++;
            }
            else
            {
                temp = this->tailRow;
                row->setNextRow(row, this->headRow);
                row->setPrevRow(row, this->tailRow->getNextRow(this->tailRow));
                this->tailRow = this->tailRow->getPrevRow(this->tailRow);
                this->headRow->setPrevRow(this->headRow, row);
                this->tailRow->setNextRow(this->tailRow, row);
                this->headRow = row;
                gui_grid_destroy_row(temp);
            }

            grid->updateRowIndex(grid);
        }
        else
        {
            row = NULL;
            TLOGMSG(1, (DBGINFOFMT "failed to create row\n", DBGINFO));
        }
    }
    else
    {
        row = NULL;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return row;
}


static struct grid_row *
gui_grid_get_row(struct gui_grid_interface *grid, int idx)
{
    struct grid_row *row = NULL;
    struct grid_row_attribute *attr = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
    {
        if (idx < this->numRows)
        {
            row = this->headRow;

            for (int i = 0; i < idx; i++)
            {
                attr = (struct grid_row_attribute *) row;
                row = attr->nextRow;
            }
        }
        else
            TLOGMSG(1, (DBGINFOFMT "invalid row index\n", DBGINFO));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));

    return row;
}


static struct grid_row *
gui_grid_get_head_row(struct gui_grid_interface *grid)
{
    struct grid_row *row = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
        row = this->headRow;
    else
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));

    return row;
}


static struct grid_row *
gui_grid_get_tail_row(struct gui_grid_interface *grid)
{
    struct grid_row *row = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
        row = this->tailRow;
    else
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));

    return row;
}


static struct grid_row *
gui_grid_get_curr_row(struct gui_grid_interface *grid)
{
    struct grid_row *row = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
        row = this->currRow;
    else
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));

    return row;
}


static struct grid_row *
gui_grid_select_prev_row(struct gui_grid_interface *grid)
{
    struct grid_row *row = NULL;
    struct grid_row_attribute *attr = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
    {
        attr = (struct grid_row_attribute *) this->currRow;

        if (attr)
        {
            this->currRow = attr->prevRow;
            row = this->currRow;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));

    return row;
}


static struct grid_row *
gui_grid_select_next_row(struct gui_grid_interface *grid)
{
    struct grid_row *row = NULL;
    struct grid_row_attribute *attr = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
    {
        attr = (struct grid_row_attribute *) this->currRow;

        if (attr)
        {
            this->currRow = attr->nextRow;
            row = this->currRow;
        }
    }
    else
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));

    return row;
}


static struct grid_row *
gui_grid_get_header_column(struct gui_grid_interface *grid)
{
    struct grid_row *colhdr = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
        colhdr = this->headerColumn;
    else
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));

    return colhdr;
}


struct gui_grid_interface *
gui_grid_create(void)
{
    struct gui_grid_interface *grid = NULL;
    struct gui_grid_attribute *this = malloc(sizeof(struct gui_grid_attribute));

    if (this)
    {
        memset(this, 0x00, sizeof(struct gui_grid_attribute));
        this->headerColumn = gui_grid_create_row();

        grid = &(this->extif);
        grid->setPosition = gui_grid_set_position;
        grid->getPosition = gui_grid_get_position;
        grid->addRow      = gui_grid_add_row;
        grid->getRow      = gui_grid_get_row;
        grid->getHeadRow  = gui_grid_get_head_row;
        grid->getTailRow  = gui_grid_get_tail_row;
        grid->getCurrRow  = gui_grid_get_curr_row;
        grid->setPrevRow  = gui_grid_select_prev_row;
        grid->setNextRow  = gui_grid_select_next_row;
        grid->getNumRows  =  gui_grid_get_nrows;
        grid->removeRow   = gui_grid_remove_row;
        grid->addHeaderColumn    = gui_grid_add_header_column;
        grid->getNumColumnHeader = gui_grid_get_number_header_columns;
        grid->getColumnHeader    = gui_grid_get_header_column;
        grid->updateRowIndex     = gui_grid_update_row_index;

        TLOGMSG(0, ("create grid widget interface (0x%p)\n", grid));
    }
    else
        TLOGMSG(1, (DBGINFOFMT "malloc return null\n", DBGINFO));

    return grid;
}


int
gui_grid_destroy(grid_t *grid)
{
    int ret = 0;
    struct grid_row *row = NULL;
    struct grid_row *temp = NULL;
    struct gui_grid_attribute *this = (struct gui_grid_attribute *)grid;

    if (this)
    {
        while(this->numRows != 0)
        {
            row = this->tailRow;
            temp = row->getPrevRow(row);
            temp->setNextRow(temp, this->headRow->getPrevRow(this->headRow));
            this->headRow->setPrevRow(this->headRow, row->getPrevRow(row));
            this->tailRow = row->getPrevRow(row);
            this->numRows--;
            gui_grid_destroy_row(row);
        }

        gui_grid_destroy_row(this->headerColumn);
        free(this);
        TLOGMSG(0, ("destroy grid widget interface\n"));
    }
    else
    {
        ret = -1;
        TLOGMSG(1, (DBGINFOFMT "null grid widget interface\n", DBGINFO));
    }

    return ret;
}