#ifndef RzxDict23432432432432432
#define RzxDict23432432432432432

template <class KEY, class VALUE> class RzxDictIterator;

template <class KEY, class VALUE> class RzxDict
{
  private:
    friend class RzxDictIterator<KEY,VALUE>;
    class Node
    {
      public:
        Node(KEY nkey, VALUE *nvalue);

	KEY key;
	VALUE *value;
	unsigned count;

	Node *left, *right;
	Node *prev, *next;

	static bool insert(Node *&root, Node *&first, Node *&last, Node *prev, 
			   Node *next, KEY nkey, VALUE *nvalue);
	static bool remove(Node *&root, Node *&first, Node *&last, KEY nkey, VALUE *&nvalue);
	static bool find(Node *&root, KEY nkey, VALUE *&nvalue);
	static bool find_nearest(Node *&rot, KEY nkey, KEY &nkey_lower, KEY &nkey_higher);
    };

    Node *root;
    Node *first;
    Node *last;

  public:
    RzxDict();
    ~RzxDict();

    void clear();

    bool insert(KEY key, VALUE *value);

    bool contains(KEY key);
    bool find(KEY key, VALUE *&value);
    bool find_nearest(KEY key, KEY &key_lower, KEY &key_higher);

    bool remove(KEY key, VALUE *&value);
    bool remove(KEY key);

    unsigned count();
};

//-------------------

template <class KEY, class VALUE> RzxDict<KEY,VALUE>::Node::Node(KEY nkey, VALUE *nvalue)
{
  key = nkey;
  value = nvalue;
  count = 1;
}

template <class KEY, class VALUE> bool
  RzxDict<KEY,VALUE>::Node::insert(RzxDict<KEY,VALUE>::Node *&root, RzxDict<KEY,VALUE>::Node *&first,
  				 RzxDict<KEY,VALUE>::Node *&last, RzxDict<KEY,VALUE>::Node *prev,
				 RzxDict<KEY,VALUE>::Node *next, KEY nkey, VALUE *nvalue)
{
  if(root==NULL)
  {
    root = new Node(nkey, nvalue);
    root->prev = prev;
    root->next = next;
    root->left = NULL;
    root->right = NULL;
    if(prev!=NULL)
      prev->next = root;
    else
      first = root;
    if(next!=NULL)
      next->prev = root;
    else
      last = root;
    return true;
  }

  if(root->key==nkey)
    return false;

  if(nkey<root->key) // Insertion à gauche
  {
    if((root->left==NULL)||((root->right!=NULL)&&(root->left->count<=root->right->count))) // Ajout normal
    {
      if(!insert(root->left, first, last, root->prev, root, nkey, nvalue))
        return false;

      root->count++;
      return true;
    }

    // Ajout forcé...
    Node *newpos = root;
    while( ((newpos->prev!=NULL)&&(newpos->prev->key>=nkey)) ||
    	   ((newpos->next!=NULL)&&(newpos->next->key<=nkey)) )
      newpos = nkey<newpos->key ? newpos->left : newpos->right;

    if(nkey==newpos->key)
      return false;

    if(newpos->key<nkey)
      newpos = newpos->next;

    KEY oldkey = root->key;
    VALUE *oldvalue = root->value;
    Node *pos1 = root, *pos2 = root->prev;
    while(pos1!=newpos)
    {
      pos1->key = pos2->key;
      pos1->value = pos2->value;
      pos1 = pos2;
      pos2 = pos1->prev;
    }
    pos1->key = nkey;
    pos1->value = nvalue;

    root->count++;
    insert(root->right, first, last, root, root->next, oldkey, oldvalue);
    return true;
  }

  // Insertion a droite

  if((root->right==NULL)||((root->left!=NULL)&&(root->right->count<=root->left->count))) // Ajout normal
  {
    if(!insert(root->right, first, last, root, root->next, nkey, nvalue))
      return false;

    root->count++;
    return true;
  }

  // Ajout forcé...
  Node *newpos = root;
  while( ((newpos->prev!=NULL)&&(newpos->prev->key>=nkey)) || 
  	 ((newpos->next!=NULL)&&(newpos->next->key<=nkey)) )
    newpos = nkey<newpos->key ? newpos->left : newpos->right;

  if(nkey==newpos->key)
    return false;

  if(newpos->key>nkey)
    newpos = newpos->prev;

  KEY oldkey = root->key;
  VALUE *oldvalue = root->value;
  Node *pos1 = root, *pos2 = root->next;
  while(pos1!=newpos)
  {
    pos1->key = pos2->key;
    pos1->value = pos2->value;
    pos1 = pos2;
    pos2 = pos1->next;
  }

  pos1->key = nkey;
  pos1->value = nvalue;

  root->count++;
  insert(root->left, first, last, root->prev, root, oldkey, oldvalue);
  return true;
}

template <class KEY, class VALUE> bool
  RzxDict<KEY,VALUE>::Node::remove(RzxDict<KEY,VALUE>::Node *&root, RzxDict<KEY,VALUE>::Node *&first,
  				 RzxDict<KEY,VALUE>::Node *&last, KEY nkey, VALUE *&nvalue)
{
  if(root==NULL)
    return false;

  if(nkey<root->key) // Deletion a gauche
  {
    if((root->left!=NULL)&&((root->right==NULL)||(root->left->count>=root->right->count)))  // Deletion normale
    {
      if(!remove(root->left,first,last,nkey,nvalue))
        return false;

      root->count--;
      return true;
    }

    // Deletion forcee
    Node *newpos = root->left;
    while((newpos!=NULL)&&(nkey!=newpos->key))
      newpos = nkey<newpos->key ? newpos->left : newpos->right;

    if(newpos==NULL)
      return false;

    nvalue = newpos->value;

    Node *pos1=newpos, *pos2=newpos->next;
    while(pos1!=root)
    {
      pos1->key = pos2->key;
      pos1->value = pos2->value;
      pos1 = pos2;
      pos2 = pos1->next;
    }

    root->count--;
    root->key = root->next->key;
    remove(root->right, first, last, root->next->key, root->value);
    return true;
  }
  
  if(root->key<nkey)  // Deletion a droite
  {
    if((root->right!=NULL)&&((root->left==NULL)||(root->right->count>=root->left->count))) // Deletion normale
    {
      if(!remove(root->right,first,last,nkey,nvalue))
        return false;

      root->count--;
      return true;
    }

    //Deletion forcee
    Node *newpos = root->right;
    while((newpos!=NULL)&&(nkey!=newpos->key))
      newpos = nkey<newpos->key ? newpos->left : newpos->right;

    if(newpos==NULL)
      return false;

    nvalue = newpos->value;

    Node *pos1=newpos, *pos2=newpos->prev;
    while(pos1!=root)
    {
      pos1->key = pos2->key;
      pos1->value = pos2->value;
      pos1 = pos2;
      pos2 = pos1->prev;
    }

    root->count--;
    root->key = root->prev->key;
    remove(root->left, first, last, root->prev->key, root->value);
    return true;
  }

  // On est bon :)
  nvalue = root->value;
  
  if(root->count==1)
  {
    if(root==first)
      first = first->next;
    else
      root->prev->next = root->next;
    if(root==last)
      last = last->prev;
    else
      root->next->prev = root->prev;

    delete root;
    root=NULL;
    return true;
  }

  root->count--;

  if((root->right==NULL)||((root->left!=NULL)&&(root->right->count<root->left->count))) // Deletion a gauche
  {
    root->key = root->prev->key;
    remove(root->left, first, last, root->prev->key, root->value);
    return true;
  }

  root->key = root->next->key;
  remove(root->right, first, last, root->next->key, root->value);
  return true;
}

template <class KEY, class VALUE> bool 
  RzxDict<KEY,VALUE>::Node::find(RzxDict<KEY,VALUE>::Node *&root, KEY nkey, VALUE *&nvalue)
{
  Node *pos = root;
  while(pos!=NULL)
  {
    if(nkey==pos->key)
    {
      nvalue = pos->value;
      return true;
    }

    pos = nkey<pos->key ? pos->left : pos->right;
  }
  return false;
}

template <class KEY, class VALUE> bool 
  RzxDict<KEY,VALUE>::Node::find_nearest(RzxDict<KEY,VALUE>::Node *&root, KEY nkey, KEY &nkey_lower, KEY &nkey_higher)
{
  Node *prev_pos = NULL;
  Node *pos = root;
  while(pos!=NULL)
  {
    if(nkey==pos->key)
    {
      nkey_lower = nkey_higher = nkey;
      return true;
    }

    prev_pos = pos;
    pos = nkey<pos->key ? pos->left : pos->right;
  }
  if(prev_pos!=NULL)
  {
    if(nkey<prev_pos->key)
    {
      nkey_higher = prev_pos->key;
      nkey_lower  = prev_pos->prev==NULL ? prev_pos->key : prev_pos->prev->key;
    }
    else
    {
      nkey_lower  = prev_pos->key;
      nkey_higher = prev_pos->next==NULL ? prev_pos->key : prev_pos->next->key;
    }
  }
  return false;
}

template <class KEY, class VALUE> RzxDict<KEY,VALUE>::RzxDict()
{
  root = first = last = NULL;
}

template <class KEY, class VALUE> RzxDict<KEY,VALUE>::~RzxDict()
{
  clear();
}

template <class KEY, class VALUE> void RzxDict<KEY,VALUE>::clear()
{
  Node *pos1, *pos2 = first;
  while(pos2!=NULL)
  {
    pos1 = pos2;
    pos2 = pos1->next;
    delete pos1->value;
    delete pos1;
  }
  root = first = last = NULL;
}

template <class KEY, class VALUE> bool RzxDict<KEY,VALUE>::insert(KEY key, VALUE *value)
{
  return Node::insert(root,first,last,NULL,NULL,key,value);
}

template <class KEY, class VALUE> bool RzxDict<KEY,VALUE>::contains(KEY key)
{
  VALUE *val;
  return Node::find(root,key,val);
}

template <class KEY, class VALUE> bool RzxDict<KEY,VALUE>::find(KEY key, VALUE *&value)
{
  return Node::find(root,key,value);
}
template <class KEY, class VALUE> bool RzxDict<KEY,VALUE>::find_nearest(KEY key, KEY &key_lower, KEY &key_higher)
{
  return Node::find_nearest(root,key,key_lower,key_higher);
}

template <class KEY, class VALUE> bool RzxDict<KEY,VALUE>::remove(KEY key, VALUE *&value)
{
  return Node::remove(root,first,last,key,value);
}

template <class KEY, class VALUE> bool RzxDict<KEY,VALUE>::remove(KEY key)
{
  VALUE *val;
  if(!Node::remove(root,first,last,key,val))
    return false;

  delete val;
  return true;
}

template <class KEY, class VALUE> unsigned RzxDict<KEY,VALUE>::count()
{
  if(root==NULL)
    return 0;
  return root->count;
}

#endif
