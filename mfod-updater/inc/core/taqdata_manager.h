#ifndef _TAQDATA_MANAGER_H_
#define _TAQDATA_MANAGER_H_

/* constant macro defines */
#define LENGTH_COORDSTR					256

#define TARGET_SHAPE_POINT              0
#define TARGET_SHAPE_CIRCLE             1
#define TARGET_SHAPE_SQUARE             2

#define PATH_TARGET_LIST                "/root/amod/target.list"

/* structure declarations : target data  */
typedef struct taqdata
{
    struct timespec taqtime;

    struct observer
    {
        int sdist;
        int hdist;
        double fwdaz;
        double fwdel;
        double gridvar;
        double gridcvg;
        double magdecl;
        double latitude;
        double longitude;
        double altitude;
        char coordstr[3][LENGTH_COORDSTR];
    }
    observer;

    struct target
    {
        char number[8];     /* target number string : alphabet 2 character + 4 digit number */
        int shape;          /* target shape */
        int length;         /* target length,  1~16,383m            */
        int width;          /* target width,   1~16,383m            */
        int attitude;       /* target attitude, 0~6399 mils         */
        int radius;         /* target radius,  1~16,383m            */
        int speed;          /* moving target speed,    0~127        */
        int course;         /* moving target course, 0~6399mils     */
        double latitude;
        double longitude;
        double altitude;
        char coordstr[3][LENGTH_COORDSTR];
    }
    target;

    struct shift
    {
        int lateral;
        int range;
        int vetical;
        double fwdaz;
    }
    shift;
}
taqdata_t;


typedef struct taqdata_manager_interface
{
    int (*addData)          (struct taqdata_manager_interface *, taqdata_t *);
    int (*removeData)       (struct taqdata_manager_interface *);
    int (*getNumData)       (struct taqdata_manager_interface *);
    int (*saveData)         (struct taqdata_manager_interface *, char *);
    int (*loadData)         (struct taqdata_manager_interface *, char *);
    taqdata_t *(*createData)(struct taqdata_manager_interface *);
    taqdata_t *(*getData)   (struct taqdata_manager_interface *, int);
    taqdata_t *(*focusNext) (struct taqdata_manager_interface *);
    taqdata_t *(*focusPrev) (struct taqdata_manager_interface *);
    taqdata_t *(*getFocus)  (struct taqdata_manager_interface *);
    taqdata_t *(*setFocus)  (struct taqdata_manager_interface *, int);
    
#ifdef _ENABLE_TAQDATA_MANAGER_INTERNAL_INTERFACE_

#endif
}
taqdata_manager_t;


/* external function prototypes */
extern struct taqdata_manager_interface *taqdata_manager_create(void);
extern int taqdata_manager_destroy(struct taqdata_manager_interface *mgr);

#endif /* _TAQDATA_MANAGER_H_ */
