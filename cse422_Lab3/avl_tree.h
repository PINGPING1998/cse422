#include <stdio.h>
#include <stdlib.h>
#include <string.h>



//The AVL tree node
struct Node{
	int key;
	struct Node *left;
	struct Node *right;
	int height;
	char * content;
};

#define MALLOC_FAILED -3
#define NEWCONTENT_INVALID -2
#define ERROR_IN_SEND -4

/*A utility function to get the maximum of two integers
 * 	if a is greater than b, condition returns a - else b */
int max(int a, int b){
	return (a > b)? a : b;
}

//A utility function to get the height of the tree
int height(struct Node *N){
	if (N == NULL){
		return 0;
	}
	return N->height;
}

/*Helper function that allocates a new node with the given NULL
left and right poitners*/
struct Node * newNode(int key, char * newcontent){
	struct Node* node = (struct Node*)malloc(sizeof(struct Node));
	node->key = key;
	node->left = NULL;
	node->right = NULL;
	node->height = 1; //new node is initially added as leaf
	node->content = NULL; //initialize the content to null
	
	//add line to node
	
	//valid content
	if(strlen(newcontent) > 1){
		//allocate space
		node->content = (char *)malloc(sizeof(char) * strlen(newcontent));
		if(node->content == NULL){
			printf("error with malloc in > 1\n");
			exit(MALLOC_FAILED);
		}

		//allocation done -- copy content to the node
		strncpy(node->content, newcontent, sizeof(char)*strlen(newcontent));
	}
	if(strlen(newcontent) == 1){
		
		node->content = (char *)malloc(sizeof(char) * strlen(newcontent));
		if(node -> content == NULL){
			printf("error with malloc in == 1\n");
			exit(MALLOC_FAILED);
		}
		(node->content)[0] = '\n';
		(node->content)[1] = '\0';
	}

	//error in content
	if(strlen(newcontent) < 1){
		printf("error in getting content\n");
		exit( NEWCONTENT_INVALID);
	}

	
	return(node);
}

/* A utility function to rotate subtree rooted with y */
struct Node *rightRotate(struct Node *y){
	struct Node *x = y->left;
	struct Node *T2 = x->right;

	//preform rotation
	x->right = y;
	y->left = T2;

	//Update heights
	y->height = max(height(y->left), height(y->right))+1;
	x->height = max(height(x->left), height(x->right)) + 1;

	//Return new root
	return x;
}

/* a utility function to rotate subtreee rooted with x */
struct Node *leftRotate(struct Node *x){
	struct Node *y = x->right;
	struct Node *T2 = y->left;

	//Perform rotation
	y->left = x;
	x->right = T2;

	//Update heights
	x->height = max(height(x->left), height(x->right)) + 1;
	y->height = max(height(y->left), height(y->right)) + 1;

	//return new root
	return y;
}

//Get balance factor of node N
int getBalance(struct Node *N){
	if(N == NULL){
		return 0;
	}
	return height(N->left) - height(N->right);
}

// Recursive function to insert a key in the subtree
// rooted with node and returns the new root of the subtree
struct Node * insert(struct Node * node, int key, char * newcontent){

	/*1. Perform the nomral BST insertion */
	if(node == NULL){
		return (newNode(key, newcontent));
	}

	if(key < node->key){ 
		node->left = insert(node->left, key, newcontent);
	}
	else if (key > node->key){
		node->right = insert(node->right, key, newcontent);
	}
	else{ // Equal keys are not allowed in the BST
		return node;
	}

	/*2. Update height of this ancestor node */
	node->height = 1 + max(height(node->left), height(node->right));

	/*3. Get the balance factor of this acnestor node to check whether
	 * this node became unbalnced */
	int balance = getBalance(node);

	/*If this node becomes unbalanced, then there are 4 cases*/
	
	//Left Left Case
	if(balance > 1 && key < node->left->key){
		return rightRotate(node);
	}

	//Right Right Case
	if(balance < -1 && key > node->right->key){
		return leftRotate(node);
	}

	//Left Right Case
	if (balance > 1 && key > node->left->key){
		node->left = leftRotate(node->left);
		return rightRotate(node);
	}

	//Right Left Case
	if(balance < -1 && key < node->right->key){
		node->right = rightRotate(node->right);
		return leftRotate(node);
	}

	/* return the unchanged node pointer */
	return node;
}

void preOrder(struct Node * root){
	if(root != NULL){
		printf("%d: %s", root->key, root->content);
		preOrder(root->left);
		preOrder(root->right);
	}
}

void Order(struct Node * root){
	int z;
	if(root != NULL){
		Order(root->left);
		int lengthContent = strlen(root->content);
		z = lengthContent - 1;
		if(lengthContent > 1){
			if((root->content)[z] != '\n'){
				//If the last character is not newline then null term after middle \n
				for(z = lengthContent-1; z > 0; z--){
					if((root->content)[z] == '\n'){
						(root->content)[z+1] = '\0';
						break;
					}
				}
			}
			
			if((root->content)[z-1] == '\n'){
				if((root->content)[z] == '\n'){
					(root->content)[z] = '\0';
				}

			}
		}

			printf("%d: %s", root->key, root->content);
			fflush(stdout);
			Order(root->right);
	}
}




void WriteOrSend(struct Node * root, int isSend, int sock, FILE * fd){
	//status variables
	int status = 0;
	int z = 0;
	
	if(root != NULL){
		WriteOrSend(root->left, isSend, sock, fd);

		int lengthContent = strlen(root->content);
		z = lengthContent - 1;
		if(lengthContent > 1){
			if((root->content)[z] != '\n'){
				//If the last character is not newline then null term after middle \n
				for(z=lengthContent -1; z > 0; z--){
					if((root->content)[z] == '\n'){
						(root->content)[z+1] = '\0';
						break;
					}
				}
			}

			if(isSend==0){
				if((root->content)[z-1] == '\n'){
					if((root->content)[z] == '\n'){
						(root->content)[z] = '\0';
					}
				}
			}
			
		}




		if(isSend == 1){
			status = write(sock, root->content, lengthContent);
			if(status < 0){
				perror("error sending back data\n");
				exit(ERROR_IN_SEND);
			}

			WriteOrSend(root->right, isSend, sock, fd);
		}
		else{
			printf("%d: %s", root->key, root->content);
			fflush(stdout);
			status = fprintf(fd, "%s", root->content);
			if(status < 0){
				perror("error wrtting data\n");
				exit(ERROR_IN_SEND);
			}
			WriteOrSend(root->right, isSend, sock, fd);
			
		}
			
	}
}

void freeTree(struct Node * root){
	if(root != NULL) {
		freeTree(root->left);
		freeTree(root->right);
		free(root);
	}
}
					
