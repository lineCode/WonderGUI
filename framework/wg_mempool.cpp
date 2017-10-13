
#include <wg_mempool.h>
#include <stdlib.h>

Uint32	WgMemPool::g_allocatedEver = 0;

//____ Constructor ____________________________________________________________

WgMemPool::WgMemPool( Uint32 entriesPerBlock, Uint32 entrySize )
{
	m_nEntriesPerBlock	= entriesPerBlock;
	m_entrySize			= entrySize;
}

//____ Destructor _____________________________________________________________

WgMemPool::~WgMemPool()
{
}

//____ AllocEntry() ___________________________________________________________

void * WgMemPool::AllocEntry()
{
	g_allocatedEver++;

	Block * pBlock = m_blocks.First();
	if(pBlock == 0)
		pBlock = _addBlock();

	if( pBlock->nAllocEntries == pBlock->maxEntries )
	{
		m_blocks.PushBack(pBlock);			// This block is full so we put it in the back.

		pBlock = m_blocks.First();
		if( pBlock->nAllocEntries == pBlock->maxEntries )
		{
			_addBlock();					// We don't have any free entries left in any block.
											// so we need to create a new one.
			pBlock = m_blocks.First();
		}
	}

	return pBlock->allocEntry();
}

//____ FreeEntry() ____________________________________________________________

void WgMemPool::FreeEntry( void * pEntry )
{
	if( pEntry == 0 )
		return;

	Block * pBlock = m_blocks.First();

	while( (pBlock && pEntry < pBlock->pMemBlock) || (pEntry >= ((Uint8*)pBlock->pMemBlock) + pBlock->blockSize) )
	{
		pBlock = pBlock->Next();
	}

	if( !pBlock )
	{
		return;								// ERROR!!! ENTRY HAS NOT BEEN RESERVED THROUGH US!!!!!!!!!!
	}

	if( pBlock->nAllocEntries == pBlock->maxEntries )
		m_blocks.PushFront(pBlock);			// Full block will get an entry free, needs to be among the free ones...

	pBlock->freeEntry(pEntry);

	if( pBlock->nAllocEntries == 0 )
		delete pBlock;
}

//____ _addBlock() _____________________________________________________________

WgMemPool::Block *WgMemPool::_addBlock()
{
	Block * pBlock = new Block( m_nEntriesPerBlock, m_entrySize );
	m_blocks.PushFront( pBlock );
	return pBlock;
}

//____ Block::Constructor _____________________________________________________

WgMemPool::Block::Block( Uint32 _nEntries, Uint32 _entrySize )
{
	pMemBlock		= malloc( _nEntries*_entrySize );
	blockSize		= _nEntries*_entrySize;
	nAllocEntries	= 0;
	nCleanEntries	= 0;
	maxEntries		= _nEntries;
	firstFreeEntry	= 0;
	entrySize		= _entrySize;
}

//____ Block::Destructor ______________________________________________________

WgMemPool::Block::~Block()
{
	free( pMemBlock );
}

//____ Block::allocEntry() ____________________________________________________

void * WgMemPool::Block::allocEntry()
{
	if( nAllocEntries == maxEntries )
		return 0;

	void * p = ((Uint8*)pMemBlock) + firstFreeEntry*entrySize;
	nAllocEntries++;

	if( firstFreeEntry == nCleanEntries )
	{
		firstFreeEntry++;
		nCleanEntries++;
	}
	else
	{
		firstFreeEntry = * ((Uint32*)p);
	}
	return p;
}

//____ Block::freeEntry() _____________________________________________________

bool WgMemPool::Block::freeEntry( void * pEntry )
{
	if( pEntry < pMemBlock || pEntry >= ((Uint8*)pMemBlock) + blockSize )
		return false;

	* ((Uint32*)pEntry) = firstFreeEntry;
	firstFreeEntry = ((Uint32) (((char*)pEntry) - ((char*)pMemBlock))) / entrySize;

	nAllocEntries--;
	return true;
}


