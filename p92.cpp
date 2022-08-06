/*
A small project to solve Problem 92 from Project Euler: https://projecteuler.net/problem=92

The problem is described as follows:

"""
A number chain is created by continuously adding the square of the digits in a number to form a new number until it has been seen before.

For example,

44 → 32 → 13 → 10 → 1 → 1
85 → 89 → 145 → 42 → 20 → 4 → 16 → 37 → 58 → 89

Therefore any chain that arrives at 1 or 89 will become stuck in an endless loop. What is most amazing is that EVERY starting number will eventually arrive at 1 or 89.

How many starting numbers below ten million will arrive at 89?
"""

This program is compiled with "g++ -Wall p92.cpp -o p92
For the sake of readability I'm going to define this term:
-> "squigit":
"squigit" refers to the sum of the squared digits of a given number. For example, the squigit of 89 is 145, the squigit of 20 is 4, etc.

I am also going to self-impose a constraint that I cannot do ANY pre-calculation to help, every method must start from scratch and only use mathematical concepts to help optimize, I'm not allowed to create lookup tables or pre-compute any data for each method - it has to be done from scratch for each method.
 */


/*

IDEAS:
- brute force and check every number, this will be terribly slow and unelegant, but we can use the results generated from the brute force method to check our results for other methods - since the brute force method will be easier to write and test.
- use dynamic programming to cache results so we don't have to repeat uneccessary work, we'll need a very large amount of space to store the data, potentially up to 50 Megabytes if we use 32 bit integers. Depending on the data structure, I'll try to allocate the memory in very large chunks, only resizing when we need to for the sake of performance.
- specfically cache *chains* of numbers, it's easy to do that, just store the starting number and ending number in a chain, and store the length of the chain, we can easily put that in triplet. This should help speed up things as we build up many chains.
- maybe find a relationship between squigits of numbers, and the numbers themselves, perhaps we could use that somehow, if a clean relationship even exists.
- start with the number 89 and work our way backwards. This would neccessarily create a tree-like structure with 89 as the root node. We can work backwards from there to discover every possible starting number that ends up as 89, I'm thinking of using a breadth-first creation/traversal since the tree levels are going to be very wide.
- we can also start with 1 and do the same thing, but simply use the results in this tree to eliminate any starting number that ends up as 1, since chains are stuck in a loop when they hit 89 or 1, only one of these numbers can be reached for any given starting number.
- look for patterns in the 1 and 89 chains and see how it can help us
- use combinatorics to eleminate duplicate combinations
 */


#include <algorithm>
#include <chrono>
#include <cstdint>
#include <iostream>
#include <vector>

using namespace std::chrono;

typedef uint32_t u32; // just a convenient shorthand

const u32 TEN_MILLION{10000000};


u32 squigit(u32 val);
u32 power(u32 base, u32 exp);


// base class
class Method {   
private:   
   virtual u32 solve() const = 0;
   
public:   
   std::string class_type{};
   void print_results() const
   {
      milliseconds time{};
   
      auto start = high_resolution_clock::now();
      u32 ans = solve();
      auto stop = high_resolution_clock::now();

      time = duration_cast<milliseconds>(stop - start);

      std::cout << class_type << " result: " << ans << ", exection time in ms: " << time.count() << '\n';
   }
};


// this is the most naive and brute-force approach, it simply loops through all ten million numbers and then checks to see if it reaches 89, and if it does, it increments a counter.
class BruteForceMethod : public Method {
private:
   u32 solve() const {
      u32 numbers_ending_with_89{0};
   
      for(u32 i = 1; i < TEN_MILLION; ++i) {
	 u32 current_chain_number{i};

	 u32 chain_length{0};
	 while(true) {
	    u32 val = squigit(current_chain_number);
	 
	    if(val == 89) {
	       ++numbers_ending_with_89;
	       break;
	    } else if(val == 1) {
	       break;
	    } else {
	       current_chain_number = val;
	    }

	    ++chain_length;
	 }

	 
	 // I wanted to see how long a chain was, meaning how many numbers after a given starting number does it take to get to 89 or 1, apparently it's under 12 because this statement doesn't execute at all for all 9999999 numbers we check - uncomment the next line so this code can run.
	 // #define TEST_CHAIN
	 #ifdef TEST_CHAIN
	 if(chain_length >= 12) {
	    std::cout << "chain length: " << chain_length << "\n";
	 }
	 #endif
      }

      return numbers_ending_with_89;
   }

public:
   BruteForceMethod()
   {
      class_type = std::string{"brute_force_method"};
   }
};


// This is similar to the above method, but it also makes use of a cache that is updated every loop iteration to include numbers that we know will reach 89, which speeds things up a bit.
class BruteForceMethodCached : public Method {
private:
   u32 solve() const
   {
      std::vector<bool> numbers_that_goto_89{false}; // technically "squigits" that go to 89
      std::vector<bool> numbers_that_goto_1{false};
      numbers_that_goto_89.reserve(567); // only need 9^2 * 7 values for squigits since it is 7 digits or under, this idea was not mine however, but it's useful.
      numbers_that_goto_1.reserve(567);
      u32 numbers_ending_with_89{0};
   
      for(u32 i = 1; i < TEN_MILLION; ++i) {
	 
	 u32 current_chain_number{i};
	 std::vector<u32> chain_numbers{}; // if this chain ends in 89, we will cache all these numbers in a sorted vector, otherwise if they go to 1 we'll put them in a different vector
	 bool goes_to_1{false};

	 while(true) {
	    u32 val = squigit(current_chain_number);
	 
	    if(val == 89 || numbers_that_goto_89[val]) {
	       ++numbers_ending_with_89;
	       break;
	    } else if(val == 1 || numbers_that_goto_1[val]) {
	       goes_to_1 = true;
	       break;
	    } else {
	       current_chain_number = val;
	       chain_numbers.push_back(val);
	    }
	 }

	 if(goes_to_1) {
	    for(u32 i = 0; i < chain_numbers.size(); ++i) {
	       numbers_that_goto_1[chain_numbers[i]] = true;
	    }
	 } else {
	    for(u32 i = 0; i < chain_numbers.size(); ++i) {
	       numbers_that_goto_89[chain_numbers[i]] = true;
	    }
	 }
      }

      return numbers_ending_with_89;
   }
public:
   BruteForceMethodCached()
   {
      class_type = std::string{"brute_force_method_cached"};
   }
};


/*
This method is similar to BruteForceMethodCached, but it creates the cache of numbers beforehand using the trick that all the possible squigits of 1 to 9999999 is 9^2 * 7 = 567, so we only need to check 567 values which is pretty small, and then we can cache the result immediately and then just loop through all ten million values - it's prety much the same as BruteForcedMethodCached but slightly different - and it turns out that it helps speed things up a little bit, possibly because we aren't constantly adding to a cache each loop iteration, but just perfoming a simple lookup with the index of the number we want to check.
 */
class SquigitsMethod : public Method {
private:
   
   u32 solve() const
   {
      bool squigits_to_1[1024]; // since there are much less squigits that go to 1, we simply only keep track of these, and if a number's squigit is NOT in here, then it goes to 89, so it should be fast to check this small array.

      for(u32 i = 1; i <= 567; ++i) {
	 u32 val{i};
	 while(true) {
	    if(val == 1) {
	       squigits_to_1[i] = true;
	       break;
	    }
	    if(val == 89) {
	       squigits_to_1[i] = false;
	       break;
	    }
	    val = squigit(val);
	 }
      }

      u32 ans{0};
      for(u32 i = 1; i < TEN_MILLION; ++i) {
	 if(!squigits_to_1[squigit(i)]) {
	    ++ans;
	 }
      }

      return ans;
   }
   
public:
   
   SquigitsMethod()
   {
      class_type = std::string{"squigits_method"};
   }
};


/*
All the other methods have been ignoring the fact that several combinations of numbers produce the same squigit, such as: [10, 1000, 1000], or [57, 705, 7005, 5007]
We can exploit this by enumerating all possible combinations and then checking to see if a given combination reaches 89 eventually, the trick will be to figure out how many starting numbers a given combination corresponds to. We also need to figure out how to enumerate all possible combinations, NOT permutations, which should shrink how many iterations our main loop will be by a large percentage.

Inspired from the recursive method described here: "https://stackoverflow.com/questions/12991758/creating-all-possible-k-combinations-of-n-items-in-c", we generate all combinations and then iterate over them, which turns out to be 11440 combinations! 11440 is much less than 10,000,000 to loop over - and then with each combination we compute to see if it goes to 89 using the squigit method, and then if it does, we generate the permutations from this combination which gives us how many starting numbers this combination permutates to.

Once we have all combinations, we'll need to then figure out if it runs to 89 (that's easy, we'll use the squigits method), and then we'll need to figure out how many permutations we can make from these combinations
We don't have a clean permutation formula because repeated contiguous elements are indistinguishable, such as 1111. It doesn't matter what order those 1's are in, it equals the same. But using this post on math stack exchange: "https://math.stackexchange.com/questions/2005441/possible-numbers-from-given-numbers-using-permutations-and-combinaitions", I figured out how to calulate the permutations using the "rule of product", and with this we have our DigitsMethod which is the fastest and most efficient I was able to come up with.
 */
class DigitsMethod : public Method {
   
private:

   // simple function for printing vector
   static void print_vector(const std::vector<u32> &vec)
   {
      std::cout << "VECTOR -> { ";
      for(u32 num : vec) {
	 std::cout << num << ",";
      }
      std::cout << " }\n";
   }

   // prints a 2D vector
   static void print_2D_vector(const std::vector<std::vector<u32>> &vec)
   {
      for(std::vector<u32> v : vec) {
	 print_vector(v);
      }
   }

   // helper for calculating factorial of a number
   static u32 fact(u32 num)
   {
      u32 ans{1};
      while(num != 1) {
	 ans *= num;
	 --num;
      }
      return ans;
   }
   
   // simple helper method for removing specific element in a vector, only removes first occurence of element, and returns a copy of the new array without the element (I could have also passed in a reference and modified the vector in-place, but while I was working on this DigitsMethod I though it would be better to have a return-by-copy in case I needed it)
   static std::vector<u32> remove_element(std::vector<u32> nums, u32 val)
   {      
      for(u32 i = 0; i < nums.size(); ++i) {
	 if(nums[i] == val) {
	    nums.erase(nums.begin() + i);
	    return nums;
	 }
      }
      return nums;
   }
   
   // creates all combinations of elements with r choose elements
   static std::vector<std::vector<u32>> combination(std::vector<u32> elements, u32 r) 
   {
      if(r == 1) {
	 std::vector<std::vector<u32>> ans{};
	 for(auto element : elements) {
	    ans.push_back(std::vector<u32>{element});
	 }
	 return ans;
      } else {
	 std::vector<std::vector<u32>> return_val{};

	 std::vector<u32> elements_to_use = elements;
	 for(auto element : elements) {
	    auto combos = combination(elements_to_use, r - 1);
	    
	    for(auto combo : combos) {
	       combo.push_back(element);
	       return_val.push_back(combo);
	    }

	    elements_to_use = remove_element(elements_to_use, element);
	 }	
	 
	 return return_val;
      }
   }
   
   u32 solve() const
   {
      bool squigits_to_1[1024];

      for(u32 i = 1; i <= 567; ++i) {
	 u32 val{i};
	 while(true) {
	    if(val == 1) {
	       squigits_to_1[i] = true;
	       break;
	    }
	    if(val == 89) {
	       squigits_to_1[i] = false;
	       break;
	    }
	    val = squigit(val);
	 }
      }

      
      std::vector<u32> nums{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
      std::vector<std::vector<u32>> combinations = combination(nums, 7);

      
      u32 ans{0};
      
      for(auto combo : combinations) {
	 u32 squigit_val{0};
	 for(u32 num : combo) {
	    squigit_val += squigit(num);
	 }

	 // if we get a number running to 89, we then calculate all permutations this combo will make using the rule of product, also we're checking to see if squigit_val is not 0 because if it is it will add another value to our 89 count which would be a bug
	 if(squigit_val != 0 && !squigits_to_1[squigit_val]) {
	    u32 starting_numbers{0};

	    starting_numbers = fact(combo.size());

	    int duplicates[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
	    for(u32 val : combo) {
	       ++duplicates[val];
	    }

	    for(u32 i = 0; i < 10; ++i) {
	       if(duplicates[i] > 0) {
		  starting_numbers = starting_numbers / fact(duplicates[i]);
	       }
	    }

	    ans += starting_numbers;
	 }
      }
      
      return ans;
   }
public:
   DigitsMethod()
   {
      class_type = std::string{"digits_method"};
   }
};


int main()
{
   std::cout << "Solving Problem 92" << '\n';


   const BruteForceMethod brute_force_method{};
   const BruteForceMethodCached brute_force_method_cached{};
   const SquigitsMethod squigits_method{};
   const DigitsMethod digits_method{};

   brute_force_method.print_results(); // brute force method takes about 3 seconds on my machine, fairly slow
   
   brute_force_method_cached.print_results(); // cached method seems to be about twice as fast, taking about 1.5 seconds on my machine
   
   squigits_method.print_results(); // this method is just slighlty faster than the previous, I usually get about a 350 millisecond difference on average

   digits_method.print_results(); // this method is IT! it is staggeringly fast compared to the others, and probably scales much better. On average it takes 45ms to run this!! Which is a HUGE improvement.

   
   /* 
Interestingly, when I enable -O2 in the compiler, the difference between BruteForceMethodCached and SquigitsMethod shrinks to almost nothing, about only a 5-10ms difference on average. I'm not sure what the compiler is doing to achieve that but they both boil out to be close to the same speed with gcc -O2 optimization level. (SquigitsMethod is just every so slightly faster though)
Also, DigitsMethod only takes 6ms on average!! Which is less than 1% of a second!!
   */
  
   
   /*
THINGS I LEARNED:
- it turns out for the cached method we only need a cache size of 567 elements because 9^2 * 7 = 567, so we don't need a massive cache size like I thought before, which would have made it way too slow.
- under ten million, chains are at most 12 long, including the starting number.
- trying to create an inverse (squigit) was difficult, I couldn't find a way to efficiently calculate a list of numbers that squigit to X without looping over all ten million numbers and checking, we might as well just bruteforce it if that's the case - I was going to try to make a tree with 89 or 1 at the root but that fell apart, perhaps it could be done but I don't know if it could possibly be better than the DigitsMethod.
- combinatorics is useful for eliminating the amount of cases you have to deal with, especially for large sets of data, which is a very huge optimization in this case.
    */
   
   return 0;
}


u32 power(u32 base, u32 exp)
{
   u32 ans{1};
   while(exp != 0) {
      ans *= base;
      --exp;
   }
   return ans;
}


u32
squigit(u32 val)
{
   u32 ans{0};
   
   u32 i = 1;
   while(val != 0) {
      u32 mod_val = val % power(10, i);
      val -= mod_val;
      ans += (mod_val / power(10, i - 1)) * (mod_val / power(10, i - 1));
      ++i;
      // std::cout << "val: " << val << ", ans: " << ans << ", mod val:" << mod_val << "\n";
   }
   
   return ans;
}
