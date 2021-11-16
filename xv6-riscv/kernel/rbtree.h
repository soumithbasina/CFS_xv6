typedef struct red_black_tree
{
    struct spinlock lock;

    int nproc;
    uint64 min_vruntime;
    struct proc *root;
    struct proc *NIL;
} red_black_tree;

red_black_tree rq;
struct proc nil_node;

// called in userinit() in proc.c
void new_red_black_tree()
{
    nil_node.left = 0;
    nil_node.right = 0;
    nil_node.par = 0;
    nil_node.color = Black;
    initlock(&nil_node.lock, "nil");
    rq.NIL = &nil_node;
    rq.root = rq.NIL;
    rq.nproc = 0;
    rq.min_vruntime = 0;
    initlock(&rq.lock, "rq");
}

struct proc *leftmostnode(red_black_tree *t)
{
    struct proc *temp = t->root;
    while (temp && temp != t->NIL && temp->left != t->NIL)
    {
        temp = temp->left;
    }
    return temp;
}

void left_rotate(red_black_tree *t, struct proc *x)
{
    struct proc *y = x->right;
    x->right = y->left;
    if (y->left != t->NIL)
    {
        y->left->par = x;
    }
    y->par = x->par;
    if (x->par == t->NIL)
    { //x is root
        t->root = y;
    }
    else if (x == x->par->left)
    { //x is left child
        x->par->left = y;
    }
    else
    { //x is right child
        x->par->right = y;
    }
    y->left = x;
    x->par = y;
}

void right_rotate(red_black_tree *t, struct proc *x)
{
    struct proc *y = x->left;
    x->left = y->right;
    if (y->right != t->NIL)
    {
        y->right->par = x;
    }
    y->par = x->par;
    if (x->par == t->NIL)
    { //x is root
        t->root = y;
    }
    else if (x == x->par->right)
    { //x is left child
        x->par->right = y;
    }
    else
    { //x is right child
        x->par->left = y;
    }
    y->right = x;
    x->par = y;
}

void insertion_fixup(red_black_tree *t, struct proc *z)
{
    while (z->par->color == Red)
    {
        if (z->par == z->par->par->left)
        { //z.par is the left child

            struct proc *y = z->par->par->right; //uncle of z

            if (y->color == Red)
            { //case 1
                z->par->color = Black;
                y->color = Black;
                z->par->par->color = Red;
                z = z->par->par;
            }
            else
            { //case2 or case3
                if (z == z->par->right)
                {               //case2
                    z = z->par; //marked z.par as new z
                    left_rotate(t, z);
                }
                //case3
                z->par->color = Black;    //made par black
                z->par->par->color = Red; //made par red
                right_rotate(t, z->par->par);
            }
        }
        else
        {                                       //z.par is the right child
            struct proc *y = z->par->par->left; //uncle of z

            if (y->color == Red)
            {
                z->par->color = Black;
                y->color = Black;
                z->par->par->color = Red;
                z = z->par->par;
            }
            else
            {
                if (z == z->par->left)
                {
                    z = z->par; //marked z.par as new z
                    right_rotate(t, z);
                }
                z->par->color = Black;    //made par black
                z->par->par->color = Red; //made par red
                left_rotate(t, z->par->par);
            }
        }
    }
    t->root->color = Black;
}

void insert(red_black_tree *t, struct proc *z)
{
    struct proc *y = t->NIL; //variable for the par of the added node
    struct proc *temp = t->root;

    if (t->min_vruntime > z->vruntime)
        t->min_vruntime = z->vruntime;

    while (temp != t->NIL)
    {
        y = temp;
        if (z->vruntime <= temp->vruntime)
            temp = temp->left;
        else
            temp = temp->right;
    }
    z->par = y;

    if (y == t->NIL)
    { //newly added node is root
        t->root = z;
    }
    else if (z->vruntime <= y->vruntime) //data of child is less than its par, left child
        y->left = z;
    else
        y->right = z;

    z->right = t->NIL;
    z->left = t->NIL;

    insertion_fixup(t, z);
    t->nproc++;
}

void rb_transplant(red_black_tree *t, struct proc *u, struct proc *v)
{
    if (u->par == t->NIL)
        t->root = v;
    else if (u == u->par->left)
        u->par->left = v;
    else
        u->par->right = v;
    v->par = u->par;
}

struct proc *minimum(red_black_tree *t, struct proc *x)
{
    while (x != 0 && x != t->NIL && x->left != t->NIL)
        x = x->left;
    return x;
}

void rb_delete_fixup(red_black_tree *t, struct proc *x)
{
    while (x != t->root && x->color == Black)
    {
        if (x == x->par->left)
        {
            struct proc *w = x->par->right;
            if (w->color == Red)
            {
                w->color = Black;
                x->par->color = Red;
                left_rotate(t, x->par);
                w = x->par->right;
            }
            if (w->left->color == Black && w->right->color == Black)
            {
                w->color = Red;
                x = x->par;
            }
            else
            {
                if (w->right->color == Black)
                {
                    w->left->color = Black;
                    w->color = Red;
                    right_rotate(t, w);
                    w = x->par->right;
                }
                w->color = x->par->color;
                x->par->color = Black;
                w->right->color = Black;
                left_rotate(t, x->par);
                x = t->root;
            }
        }
        else
        {
            struct proc *w = x->par->left;
            if (w->color == Red)
            {
                w->color = Black;
                x->par->color = Red;
                right_rotate(t, x->par);
                w = x->par->left;
            }
            if (w->right->color == Black && w->left->color == Black)
            {
                w->color = Red;
                x = x->par;
            }
            else
            {
                if (w->left->color == Black)
                {
                    w->right->color = Black;
                    w->color = Red;
                    left_rotate(t, w);
                    w = x->par->left;
                }
                w->color = x->par->color;
                x->par->color = Black;
                w->left->color = Black;
                right_rotate(t, x->par);
                x = t->root;
            }
        }
    }
    x->color = Black;
    rq.min_vruntime = leftmostnode(&rq)->vruntime;
}

void rb_delete(red_black_tree *t, struct proc *z)
{
    struct proc *y = z;
    struct proc *x;
    enum COLOR y_orignal_color = y->color;
    if (z->left == t->NIL)
    {
        x = z->right;
        rb_transplant(t, z, z->right);
    }
    else if (z->right == t->NIL)
    {
        x = z->left;
        rb_transplant(t, z, z->left);
    }
    else
    {
        y = minimum(t, z->right);
        y_orignal_color = y->color;
        x = y->right;
        if (y->par == z)
        {
            x->par = z;
        }
        else
        {
            rb_transplant(t, y, y->right);
            y->right = z->right;
            y->right->par = y;
        }
        rb_transplant(t, z, y);
        y->left = z->left;
        y->left->par = y;
        y->color = z->color;
    }
    if (y_orignal_color == Black)
        rb_delete_fixup(t, x);
    t->nproc--;
}
