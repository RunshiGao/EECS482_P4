/ (type d) (inode block 0)
	owner: 
	size: 1
	data disk blocks: 2 
	entry 0: apple, inode block 1

/apple (type d) (inode block 1)
	owner: user1
	size: 1
	data disk blocks: 4 
	entry 0: pear, inode block 3

/apple/pear (type d) (inode block 3)
	owner: user1
	size: 1
	data disk blocks: 6 
	entry 0: 1, inode block 5
	entry 1: 2, inode block 7
	entry 2: 3, inode block 8
	entry 3: 4, inode block 9
	entry 4: 5, inode block 10
	entry 5: 6, inode block 11
	entry 6: 7, inode block 12

/apple/pear/1 (type f) (inode block 5)
	owner: user1
	size: 0
	data disk blocks: 


/apple/pear/2 (type f) (inode block 7)
	owner: user1
	size: 0
	data disk blocks: 


/apple/pear/3 (type f) (inode block 8)
	owner: user1
	size: 0
	data disk blocks: 


/apple/pear/4 (type f) (inode block 9)
	owner: user1
	size: 0
	data disk blocks: 


/apple/pear/5 (type f) (inode block 10)
	owner: user1
	size: 0
	data disk blocks: 


/apple/pear/6 (type f) (inode block 11)
	owner: user1
	size: 0
	data disk blocks: 


/apple/pear/7 (type f) (inode block 12)
	owner: user1
	size: 0
	data disk blocks: 


4083 disk blocks free
