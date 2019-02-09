//Temple A. Corson IV
//te789951
//DUE: 12/04/2016

#include "HashyHash.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/* provided struct for refrence
typedef struct HashTable
{
	// Your hash table will store integer keys in this array.
	int *array;

	// The current capacity of your hash table (the length of 'array').
	int capacity;

	// The size of your hash table (the number of elements it contains).
	int size;

	// A pointer to the hash function for this hash table (initially NULL).
	unsigned int (*hashFunction)(int);

	// Probing type: LINEAR or QUADRATIC. Initialize to LINEAR by default.
	ProbingType probing;

	// A struct within a struct for maintaining stats on this hash table:
	// number of operations performed and number of collisions encountered.
	HashStats stats;

} HashTable;


*/

//provided prime function
/////////////////////////////////////////////////////////
// Finds the next prime number greater than or equal to n. This function assumes
// that n is strictly greater than 2. If n is exactly equal to 2, this function
// actually returns 3.
int nextPrime(int n)
{
	int i, root, keepGoing = 1;

	if (n % 2 == 0)
		++n;

	while (keepGoing)
	{
		keepGoing = 0;
		root = sqrt(n);

		for (i = 3; i <= root; i++)
		{
			if (n % i == 0)
			{
				// Move on to the next candidate for primality. Since n is odd, we
				// don't want to increment it by 1. That would give us an even
				// integer greater than 2, which would necessarily be non-prime.
				n += 2;
				keepGoing = 1;

				// Leave for-loop. Move on to next iteration of while-loop.
				break;
			}
		}
	}

	return n;
}
///////////////////////////////////////////////////////////

//functions

HashTable *makeHashTable(int capacity)
{
    int i;

    //initialize and make space for hashtable
    HashTable *newTable = malloc(sizeof(HashTable));

    //initalize array
    if(capacity < 1)
        capacity = DEFAULT_CAPACITY;

    //other initalizations
    newTable->capacity = capacity;
    newTable->array = malloc(sizeof(int) * capacity);
    newTable->size = 0;
    newTable->probing = LINEAR;                         //default

    //initalize hashstats
    newTable->stats.collisions = 0;
    newTable->stats.opCount = 0;

    //mark cells as unused
    for(i=0;i<capacity;i++)
        newTable->array[i] = UNUSED;

    return newTable;
}


HashTable *destroyHashTable(HashTable *h)
{
    if(h == NULL)
        return NULL;

    free(h->array);
    free(h);

    return NULL;
}


int setProbingMechanism(HashTable *h, ProbingType probing)
{
    if (h == NULL)
        return HASH_ERR;

    if(!((probing == LINEAR) || (probing == QUADRATIC)))
        return HASH_ERR;

    h->probing = probing;

    return HASH_OK;
}


int setHashFunction(HashTable *h, unsigned int (*hashFunction)(int))
{
    //return err if null table
    if(h == NULL)
        return HASH_ERR;

        //return ok early to avoid derefrencing a null pointer
    if(hashFunction == NULL)
        return HASH_OK;

    h->hashFunction = *hashFunction;
    return HASH_OK;
}



int isAtLeastHalfEmpty(HashTable *h)
{
    int status = 0;
    int capacity;
    int size;

    //check if null before derefrencing
    if(h == NULL)
    return status = 0;

    capacity = h->capacity;
    size = h->size;

    if(size <= (capacity / 2))
        return status = 1;

    return status;
}


int expandHashTable(HashTable *h)
{
    int oldCapacity, tempKey, i;
    int flag = 0;
    int *tempArray;

    //error checks
    if(h == NULL)
        return HASH_ERR;

    //store old capacity
    oldCapacity = h->capacity;

    //store old array for use
    tempArray = h->array;

    //reset cap for linear
    if(h->probing == LINEAR)
        h->capacity = (oldCapacity * 2) + 1;

    //reset cap for quad
    else if(h->probing == QUADRATIC)
        h->capacity = nextPrime(((oldCapacity * 2) + 1));

    else
        return HASH_ERR;        //probing is broken

    //allocate new array in h
    h->array = malloc(sizeof(int) * h->capacity);

    //mark cells in new h array as unused
    for(i=0;i<(h->capacity);i++)
        h->array[i] = UNUSED;

    //lets re-hash this
    for(i=0;i<oldCapacity;i++)
    {
        //store the key thats in array i for use
        tempKey = tempArray[i];

        //if spot in old array is unused or dirty
        //then we don't want to rehash that
        if((tempKey != DIRTY) && (tempKey != UNUSED))
        {
        //insert
        flag = insert(h, tempKey);

        //we dont want to increment opcount for insertions done
        //while expanding the array so for each insert
        //decrement the opcount, because insert increments it
        //likewise with the size of the table.
        h->stats.opCount--;
        h->size--;

        //if insert fails
        if(flag == HASH_ERR)
            return HASH_ERR;
        }
    }

    //free old array
    free(tempArray);

    return HASH_OK;
}

int insert(HashTable *h, int key)
{
    int hashValue, capacity, i, j;
    int k = 1;

    if(h == NULL)
        return HASH_ERR;

    //make sure there is room
    if(!(isAtLeastHalfEmpty(h)))
    {
        i = expandHashTable(h);

        if (i == HASH_ERR)
            return HASH_ERR;
    }

    capacity = h->capacity;

    if(h->hashFunction == NULL)
        return HASH_ERR;

    //inserty part
/////////////////////////////////////////////////////////////
    hashValue = (h->hashFunction(key) % capacity);

        if(h->probing == LINEAR)
        {
            //start at the hashed value
            j = hashValue;
            while((h->array[j] != UNUSED) && (h->array[j] != DIRTY))
            {
                //each loop means a collision so update cols
                h->stats.collisions++;

                j = (j + 1) % capacity;

                if(j == (hashValue))    //implies full table not expanding correctly
                    return HASH_ERR;
            }
            //now were out of the loop a j is an empty or dirty spot
                h->array[j] = key;
        }

        else if(h->probing == QUADRATIC)
        {
            //similar to linear except the way j is increased
            j = hashValue;
            while((h->array[j] != UNUSED) && (h->array[j] != DIRTY))
            {
                //loop is col, update
                h->stats.collisions++;

                j = ((hashValue + ((int) pow(((double) k++), 2.0))) % capacity);      // goes to next quadratic spot

                if(k > capacity)        //implies table not expanding correctly
                    return HASH_ERR;
            }
            //now j is an unused or dirty spot
            h->array[j] = key;
        }

        else    //meaning probingtype is broken
            return HASH_ERR;

    //update opCount & size
    h->stats.opCount++;
    h->size++;

    return HASH_OK;
}


int search(HashTable *h, int key)
{
    int hashValue, capacity, j;
    int atIndex = -1;
    int k = 1;

    //update opcount regardless of found status
    h->stats.opCount++;

    //return -1 for errors
    if(h == NULL)
        return -1;
    else if(h->hashFunction == NULL)
        return -1;

        //set cap
    capacity = h->capacity;

    //find and set hash value
    hashValue = (h->hashFunction(key) % capacity);

    if(h->probing == LINEAR)
    {
        j = hashValue;

        //start looking
        while(h->array[j] != key)
        {
            if(h->array[j] == UNUSED)   // if its empty key is totes not there
                return -1;

            //update collisions
            h->stats.collisions++;

            j = (j + 1) % capacity;     //increase j

            if(j == hashValue)          //entire table has been searched and not found
                return -1;
        }

        //j is now on the spot where key is
        atIndex = j;
    }

    else if(h->probing == QUADRATIC)
    {
        j = hashValue;

        //start looking
        while(h->array[j] != key)
        {

            if(h->array[j] == UNUSED)       //empty spot, leave
                return -1;

            h->stats.collisions++;          //update cols

            j = ((hashValue + ((int) pow(((double) k++), 2.0)))  % capacity);     //search next

            if(k > capacity)                //key not present
                return -1;
        }
        //j is at spot with key
        atIndex = j;
    }

    else        //probing type is broken
        return -1;

    return atIndex;
}


int delete(HashTable *h, int key)
{
    int wasAtIndex;

    //errors checks
    if(h == NULL)
        return -1;
    if(h->hashFunction == NULL)
        return -1;


    //first find where the key is
    //search will update the opcount for us
    //search will also update collisions accordingly
    wasAtIndex = search(h, key);

    if(wasAtIndex == (-1))        //key wasnt there or search failed
        return wasAtIndex;

    else                        //key is found, and then marked dirty
    {
        h->array[wasAtIndex] = DIRTY;

        //update size
        h->size--;
    }

    return wasAtIndex;
}


double difficultyRating(void)
{
    return 4.9;
}

double hoursSpent(void)
{
    return 20.4;
}


//where main would be...IF I HAD ONE!!!!!!!!!!!!!!!!111!!

////////////////////////////////////////////////////////////////////////////
//test case stuff
unsigned int hash(int key)
{
	int retval = 0;

	while (key > 0)
	{
		retval *= 37;
		retval += key % 10;
		key = key / 10;
	}

	return retval;
}

void check(HashTable *h, int capacity, int ops, int collisions)
{
	if (h->capacity != capacity || h->stats.opCount != ops || h->stats.collisions != collisions)
	{
		printf("[Expected] capacity: %d, ops: %d, collisions: %d\n", capacity, ops, collisions);
		printf("[Actual] capacity: %d, ops: %d, collisions: %d\n", h->capacity, h->stats.opCount, h->stats.collisions);
		printf("\nfail whale :(\n");
		exit(1);
	}
}

int main(void)
{
	int i;
	double avg;

	HashTable *h = makeHashTable(5);
	setHashFunction(h, hash);
	setProbingMechanism(h, QUADRATIC);

	for (i = 0; i < 100000; i++)
		insert(h, i);

	// Print hash table report.
	avg = (double)h->stats.collisions / h->stats.opCount;
	printf("Number of collisions: %d\n", h->stats.collisions);
	printf("Number of operations: %d\n", h->stats.opCount);
	printf("Average collisions per op: %0.2f\n", avg);

	if (avg < 4.0)
		printf("\nThis hash function looks pretty good!\n\n");
	else
		printf("\nThis hash function is kind of terrible.\n\n");

	// Check table stats.
	check(h, 205759, 100000, 384226);

	h = destroyHashTable(h);

	printf("Everything seems to be unfolding according to plan...\n");
	return 0;
}












