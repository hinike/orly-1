/* <orly/indy/disk/buf_block.h>

   TODO

   Copyright 2010-2014 OrlyAtomics, Inc.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License. */

#pragma once

#include <cassert>
#include <condition_variable>
#include <mutex>

#include <syslog.h>
#include <unistd.h>

#include <base/class_traits.h>
#include <base/mem_aligned_ptr.h>
#include <base/mlock.h>
#include <base/spin_lock.h>

namespace Orly {

  namespace Indy {

    namespace Disk {

      /* TODO */
      class TBufBlock {
        NO_COPY(TBufBlock);
        public:

        /* TODO */
        class TPool {
          NO_COPY(TPool);
          public:

          /* TODO */
          TPool(size_t block_size, size_t block_count = 0UL)
              : BlockSize(block_size), Blob(nullptr), FirstBlock(nullptr), NumBlocksUsed(0UL), MaxBlocks(0UL) {
            assert(block_size % getpagesize() == 0);
            if (block_count) {
              Init(block_count);
            }
          }

          /* TODO */
          size_t GetNumBlocksUsed() const {
            assert(this);
            return NumBlocksUsed;
          }

          /* TODO */
          size_t GetMaxBlocks() const {
            assert(this);
            return MaxBlocks;
          }

          /* TODO */
          void Init(size_t block_count) {
            assert(this);
            assert(MaxBlocks == 0UL);
            MaxBlocks = block_count;
            if (block_count) {
              Blob = Base::MemAlignedAlloc<TBlock>(getpagesize(), BlockSize * block_count);
              Base::MlockRaw(Blob.get(), BlockSize * block_count);
              #ifndef NDEBUG
              memset(Blob.get(), 0, BlockSize * block_count);
              #endif
              assert(Blob);
              syslog(LOG_INFO, "TBufBlock allocated [%ld] bytes for [%ld] blocks of size [%ld]", (BlockSize * block_count), block_count, BlockSize);
              TBlock
                  *prev_block = reinterpret_cast<TBlock *>(&FirstBlock),
                  *block      = Blob.get();
              for (size_t i = 0; i < block_count; ++i) {
                assert(block);
                prev_block->NextBlock = block;
                block     ->NextBlock = 0;
                prev_block = block;
                block      = reinterpret_cast<TBlock *>(reinterpret_cast<uint8_t *>(block) + BlockSize);
              }
            }
          }

          /* TODO */
          void *Alloc() {
            assert(this);
            //syslog(LOG_INFO, "BufBlock [%ld] / [%ld]", NumBlocksUsed, MaxBlocks);
            void *ptr = TryAlloc();
            while (!ptr) {
              //auto now = std::chrono::system_clock::now();
              //now += std::chrono::milliseconds(1000);
              /* wait for someone to release one for us */ {
                std::unique_lock<std::mutex> lock(Lock);
                while (NumBlocksUsed >= MaxBlocks) {
                  Cond.wait(lock/*, now*/);
                }
              }
              ptr = TryAlloc();
              //syslog(LOG_EMERG, "Bad alloc in buf block pool");
              //throw std::bad_alloc();
            }
            return ptr;
          }

          /* TODO */
          void Free(void *ptr) {
            assert(this);
            assert(ptr);
            TBlock *block = static_cast<TBlock *>(ptr);
            //Base::TSpinLock::TSoftLock lock(SpinLock);
            std::lock_guard<std::mutex> lock(Lock);
            block->NextBlock = FirstBlock;
            --NumBlocksUsed;
            FirstBlock = block;
            Cond.notify_one();
          }

          /* TODO */
          void *TryAlloc() {
            assert(this);
            //Base::TSpinLock::TSoftLock lock(SpinLock);
            std::lock_guard<std::mutex> lock(Lock);
            TBlock *block = FirstBlock;
            FirstBlock = (block ? block->NextBlock : 0);
            if (block) {
              FirstBlock = block->NextBlock;
              ++NumBlocksUsed;
            } else {
              FirstBlock = nullptr;
            }
            return block;
          }

          private:

          /* TODO */
          struct TBlock {
            /* TODO */
            TBlock *NextBlock;

          };  // TBlock

          /* TODO */
          const size_t BlockSize;

          /* TODO */
          std::unique_ptr<TBlock> Blob;

          /* TODO */
          TBlock *FirstBlock;

          /* TODO */
          //Base::TSpinLock SpinLock;
          std::mutex Lock;
          std::condition_variable Cond;

          /* TODO */
          size_t NumBlocksUsed;

          /* TODO */
          size_t MaxBlocks;

        };  // TPool

        /* TODO */
        TBufBlock() {}

        /* TODO */
        ~TBufBlock() {}

        /* TODO */
        static void *operator new(size_t /*size*/) {
          return Pool.Alloc();
        }

        /* TODO */
        static void operator delete(void *ptr) {
          Pool.Free(ptr);
        }

        /* TODO */
        static TPool Pool;

        /* TODO */
        char *GetData() const {
          assert(this);
          return &Buf;
        }

        private:

        /* TODO */
        mutable char Buf;

      };  // TBufBlock

    }  // Disk

  }  // Indy

}  // Orly
