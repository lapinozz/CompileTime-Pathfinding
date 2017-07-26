#include <type_traits>

struct IgnoreT {};
constexpr IgnoreT IgnoreV;

template<int X, int Y>
struct Vec
{
	template<int sizeX>
	static constexpr int Index = Y * sizeX + X;

	static constexpr int x = X;
	static constexpr int y = Y;

	using Up = 	  Vec<X, Y - 1>;
	using Down =  Vec<X, Y + 1>;
	using Left =  Vec<X - 1, Y>;
	using Right = Vec<X + 1, Y>;
};

template<int sizeX, int Index>
using FromIndex = Vec<Index % sizeX, Index / sizeX>;

template<int I>
struct H{};

template<typename T, T Value>
struct ValueHolder
{
	using Type = T;
	static constexpr T value = Value;
};

template <bool ForceGeneric, typename... T>
struct VectorImpl;

template<typename...T>
struct Vector : public VectorImpl<false, T...>
{
	static constexpr int Size = sizeof...(T);
};

template <bool ForceGeneric, typename... T>
struct VectorImpl
{
	template<typename...V>
	using PushFront = Vector<V..., T...>;

	template<typename...V>
	using PushBack = Vector<T..., V...>;

	template<typename V>
	struct Insert
	{
		template<typename>
		struct Impl;
		
		template<typename...Vs>
		struct Impl<Vector<Vs...>>
		{
			using Type = Vector<T..., Vs...>;
		};

		using Type = typename Impl<V>::Type;
	};

	template<typename V>
	using insert = typename Insert<V>::Type;

	struct PopFront
	{
		template<typename V, typename...Vs>
		struct Impl
		{
			using value = V;
			using vector = Vector<Vs...>;
		};

		using Impl_instance = Impl<T...>;

		using value = typename Impl_instance::value;
		using vector = typename Impl_instance::vector;
	};

	template<typename V, template<typename, typename> class Condition = std::is_same>
	struct Find
	{
		template <int CurrentIndex, typename Current, typename...I>
		struct Impl
		{
			static constexpr int Index = Condition<Current, V>::value ? CurrentIndex : Impl<CurrentIndex + 1, I...>::Index;
		};

		template <typename Current, typename...I>
		struct Impl<Vector<T...>::Size, Current, I...>
		{
			static constexpr int Index = -1;
		};

		static constexpr int Index = Impl<0, T..., IgnoreT>::Index;
	};

	template<typename V, template<typename, typename> class Condition = std::is_same>
	static constexpr int find = Find<V, Condition>::Index;

	template<template<typename> class Operation>
	struct Transform
	{
		template <typename Current, typename...Transformed>
		struct Impl;

		template <typename C, typename...Ct, typename...Ts>
		struct Impl<Vector<C, Ct...>, Ts...>
		{
			using Type = typename Impl<Vector<Ct...>, Ts..., typename Operation<C>::Type>::Type;
		};

		template <typename...Ts>
		struct Impl<Vector<>, Ts...>
		{
			using Type = Vector<Ts...>;
		};

		using Type = typename Impl<Vector<T...>>::Type;
	};

	template<template<typename> class Operation>
	using transform = typename Transform<Operation>::Type;

	template <int TargetIndex>
	struct Get
	{
		template <int Index, typename CurrentType, typename...Ts>
		struct Impl
		{
			using Type = typename Impl<Index - 1, Ts...>::Type;
		};

		template <typename CurrentType, typename...Ts>
		struct Impl <0, CurrentType, Ts...>
		{
			using Type = CurrentType;
		};

		using Type = typename Impl<TargetIndex, T...>::Type;
	};

	template<int TargetIndex>
	using get = typename Get<TargetIndex>::Type;

	template <int Index>
	struct Remove
	{
		template <int CurrentIndex, typename...Ts>
		struct Impl
		{
			using Type = typename Impl<CurrentIndex + 1, Ts...>::Type;
		};

		template <typename CurrentType, typename...Ts>
		struct Impl<Index, CurrentType, Ts...>
		{
			using Type = typename Impl<Index + 1, Ts...>::Type;
		};

		template <typename...Ts>
		struct Impl <Vector<T...>::Size, Ts...>
		{
			using Type = Vector<Ts...>;
		};

		using Type = typename Impl<0, T...>::Type;
	};

	template <int Index>
	using remove = typename Remove<Index>::Type;
};

template <typename T, T...V>
struct VectorImpl<false, ValueHolder<T, V>...> : public VectorImpl<true, ValueHolder<T, V>...>
{
	using vecImpl = VectorImpl<true, ValueHolder<T, V>...>;

	template<T...Values>
	using PushFront = Vector<ValueHolder<T, Values>..., ValueHolder<T, V>...>;

	template<T...Values>
	using PushBack = Vector<ValueHolder<T, V>..., ValueHolder<T, Values>...>;

	struct PopFront
	{
		template<T Value, T...Vs>
		struct Impl
		{
			static constexpr auto value = Value;
			using vector = Vector<ValueHolder<T, Vs>...>;
		};

		using Impl_instance = Impl<V...>;

		static constexpr auto value = Impl_instance::value;
		using vector = typename Impl_instance::vector;
	};

	template<int TargetIndex>
	struct Get
	{
		static constexpr auto Value = vecImpl::template get<TargetIndex>::value;
	};

	template<int TargetIndex>
	static constexpr auto get = Get<TargetIndex>::Value;

	template<T Value, template<typename, typename> class Condition = std::is_same>
	struct Find
	{
		template<typename Left, typename Right>
		struct ConditionWrapper;

		template<T Left, T Right>
		struct ConditionWrapper<ValueHolder<T, Left>, ValueHolder<T, Right>>
		{
			static constexpr bool value = Left == Right;
		};

		static constexpr int Index = vecImpl::template Find<ValueHolder<T, Value>, ConditionWrapper>::Index;
	};

	template<T Value, template<typename, typename> class Condition = std::is_same>
	static constexpr int find = Find<Value, Condition>::Index;
};

template<typename T, T...Values>
using VectorFromValue = Vector<ValueHolder<T, Values>...>;

template<int SizeX, typename...T>
struct Map : public Vector<T...>
{
	using GetVector = Vector<T...>;
	static constexpr int sizeX = SizeX;

	template<typename VecT>
	struct GetFromPos
	{
		static constexpr auto Value = GetVector::template Get <VecT::template Index<SizeX> >::Value;
	};

	template<typename VecT>
	static constexpr auto getFromPos = GetFromPos<VecT>::Value;
};

template<int SizeX, typename T, T...Values>
using MapFromValue = Map<SizeX, ValueHolder<T, Values>...>;

template<typename Vec1, typename Vec2>
struct Heuristic
{
	static constexpr int X3 = Vec2::x - Vec1::x;
	static constexpr int Y3 = Vec2::y - Vec1::y;
	static constexpr int Value = X3 * X3 + Y3 * Y3;
};

template<typename Vec1, typename Vec2>
static constexpr int heuristic = Heuristic<Vec1, Vec2>::Value;

template<int index, int G, int H, typename ParentType = void>
struct Node
{
	static constexpr int Index = index;

	static constexpr int g = G;
	static constexpr int h = H;
	static constexpr int Cost = G*G + H;

	using Parent = ParentType;

	using Flat = Node<index, G, H>;
};

template<typename Left, typename Right>
struct IsLess
{
	static constexpr bool Value = (Left::Cost < Right::Cost);
};

template<typename Left, typename Right, bool GetLeft>
struct SelectIf
{
	using Type = Left;
};

template<typename Left, typename Right>
struct SelectIf<Left, Right, false>
{
	using Type = Right;
};

template<typename Left, typename Right, bool GetLeft>
using selectIf = typename SelectIf<Left, Right, GetLeft>::Type;

template<typename Vector, 
typename StartValue = typename Vector::template get<0>,
int StartIndex = 0,
template<typename, typename> class IsLess = ::IsLess>
struct FindLeast
{
	template<int Index, int LeastIndex, typename Current, typename Least>
	struct Impl
	{
		using Next = typename Vector::template get<Index>;

		static constexpr bool IsLeast = IsLess<Current, Least>::Value;

		using least = selectIf<Current, Least, IsLeast>;
		static constexpr int I = IsLeast ? Index - 1 : LeastIndex;

		using Impl_instance = Impl<Index + 1, I, Next, least>;

		static constexpr int leastIndex = Impl_instance::leastIndex;
		using Type = typename Impl_instance::Type;
	};

	template<int LeastIndex, typename Current, typename Least>
	struct Impl<Vector::Size, LeastIndex, Current, Least>
	{
		static constexpr bool IsLeast = IsLess<Current, Least>::Value;

		static constexpr int leastIndex = IsLeast ? Vector::Size - 1 : LeastIndex;
		using Type = selectIf<Current, Least, IsLeast>;
	};

	using Impl_instance = Impl<0, StartIndex, StartValue, StartValue>;
	using Type = typename Impl_instance::Type;
	static constexpr int Index = Impl_instance::leastIndex;
};

template<typename Vec, int Index, bool ShouldGet, typename DefaultValue = void>
struct GetIf
{
	template<bool shouldGet, typename Ignore = IgnoreT>
	struct Impl
	{
		using Type = typename Vec::template get<Index>;
	};

	template<typename Ignore>
	struct Impl<false, Ignore>
	{
		using Type = DefaultValue;
	};

	using Type = typename Impl<ShouldGet>::Type;
};

template<typename Vec, typename Value, bool ShouldPushback>
struct PushBackIf
{
	template<bool shouldPushback = ShouldPushback, typename Ignore = IgnoreT>
	struct Impl
	{
		using Type = typename Vec::template PushBack<Value>;
	};

	template<typename Ignore>
	struct Impl<false, Ignore>
	{
		using Type = Vec;
	};

	using Type = typename Impl<>::Type;
};

template<typename T>
struct AStar;

template<int sizeX, typename...T>
struct AStar<Map<sizeX, T...>>
{
	using CurrentMap = Map<sizeX, T...>;

	static constexpr int sizeY = CurrentMap::Size / sizeX;

	template<int Index>
	using FromIndex = ::FromIndex<sizeX, Index>;

	static constexpr int StartIndex = CurrentMap::template find<'S'>;
	static constexpr int EndIndex = CurrentMap::template find<'E'>;

	using StartVec = FromIndex<StartIndex>;
	using EndVec = FromIndex<EndIndex>;

	template<typename OtherVec>
	static constexpr int  heuristic = ::heuristic<OtherVec, EndVec>;

	using StartNode = Node<StartIndex, 0, 0>;
	using EndNode = Node<EndIndex, 0, 0>;

	using OpenList = Vector<StartNode>;
	using ClosedList = Vector<>;

	template<typename Node>
	struct BuildNeighbors
	{
		using NodeVec = FromIndex<Node::Index>;

		template<typename CurrentVector = Vector<
											typename NodeVec::Up, 
											typename NodeVec::Down, 
											typename NodeVec::Left, 
											typename NodeVec::Right>, 
		typename FinalVector = Vector<>>
		struct NeighborsVec
		{
			using Type = FinalVector;
		};

		template<typename FinalVector, typename C, typename...Cs>
		struct NeighborsVec<Vector<C, Cs...>, FinalVector>
		{
			static constexpr bool IsValid = 
				C::x >= 0 && C::x < sizeX && 
				C::y >= 0 && C::y < sizeY;

			using Type = typename NeighborsVec<Vector<Cs...>, typename PushBackIf<FinalVector, C, IsValid>::Type>::Type;
		};

		template <typename...Ts>
		struct Impl;

		template <typename...Ts>
		struct Impl<Vector<Ts...>>
		{
			using Type = Vector<::Node<Ts::template Index<sizeX>, Node::g + 1, heuristic<Ts>, Node>...>;
		};

		using Type = typename Impl<typename NeighborsVec<>::Type>::Type;
	};

	template<typename Neighbors, typename OpenList, typename ClosedList>
	struct FilterNeighbors
	{
		template<typename Left, typename Right>
		struct Condition
		{
			static constexpr bool Value = Left::Cost <= Right::Cost && Left::Index == Right::Index;
		};

		template<typename Ns, int Index>
		struct ShouldKeep
		{
			static constexpr bool Value = (-1 == FindLeast<Ns, typename Neighbors::template get<Index>, -1, Condition>::Index);
		};

		template<typename Ns>
		struct ShouldKeep<Ns, Neighbors::Size>
		{
			static constexpr bool Value = false;
		};

		template<int Index, typename Ns = Neighbors>
		struct IsBlocked
		{
			static constexpr bool Value = CurrentMap::template get<Ns::template get<Index>::Index> == 1;
		};

		template<typename Ns>
		struct IsBlocked<Neighbors::Size, Ns>
		{
			static constexpr bool Value = false;
		};

		template<typename Ns = Vector<>, int Index = 0, 
		bool Keep = (
			ShouldKeep<OpenList, Index>::Value && 
			ShouldKeep<ClosedList, Index>::Value &&
			!IsBlocked<Index>::Value			)>
		struct Impl
		{
			using Type = typename Impl<typename Ns::template PushFront<typename Neighbors::template get<Index>>, Index + 1>::Type;
		};

		template<typename Ns, int Index>
		struct Impl<Ns, Index, false>
		{
			using Type = typename Impl<Ns, Index + 1>::Type;  
		};

		template<typename Ns>
		struct Impl<Ns, Neighbors::Size, true>
		{
			using Type = Ns;
		};

		template<typename Ns>
		struct Impl<Ns, Neighbors::Size, false>
		{
			using Type = Ns;
		};

		using Type = typename Impl<>::Type;
	};

	template<typename FinalNode>
	struct MakePath
	{
		template<typename NextNode = FinalNode, typename Path = Vector<>>
		struct Impl
		{
			using NewPath = typename Path::template PushFront<FromIndex<NextNode::Index>>;
			using Type = typename Impl<typename NextNode::Parent, NewPath>::Type;
		};

		template<typename Path>
		struct Impl<void, Path>
		{
			using Type = Path;
		};

		using Type = typename Impl<>::Type;
	};

	template<typename Left, typename Right>
	struct SamePositionCondition
	{
		static constexpr bool value = Left::Index == Right::Index;
	};

	template<typename Node>
	struct ExtractVec
	{
		using Type = FromIndex<Node::Index>;
	};

	template<typename Node>
	struct ExtractCost
	{
		using Type = H<Node::Cost>;
	};

	template<typename OpenList, typename ClosedList, bool IsDone = false, typename FinalNode = void>
	struct MainLoop
	{
		using LeastNode = FindLeast<OpenList>;
		using OpenList1 = typename OpenList::template remove<LeastNode::Index>;

		using Neighbors = typename BuildNeighbors<typename LeastNode::Type>::Type;
		using NeighborsToAdd = typename FilterNeighbors<Neighbors, OpenList1, ClosedList>::Type;

		static constexpr int LastNodeIndex = NeighborsToAdd::template find<EndNode, SamePositionCondition>;
		static constexpr bool Done = (-1 != LastNodeIndex);
		using LastNode = typename GetIf<NeighborsToAdd, LastNodeIndex, Done>::Type;

		using OpenList2 = typename OpenList1::template insert<NeighborsToAdd>;

		using ClosedList1 = typename ClosedList::template PushBack<typename LeastNode::Type::Flat>;

		using Path = typename MainLoop<OpenList2, ClosedList1, Done, LastNode>::Path;
	};

	template<typename OpenList, typename ClosedList, typename FinalNode>
	struct MainLoop<OpenList, ClosedList, true, FinalNode>
	{
		using Path = typename MakePath<FinalNode>::Type; 
	};

	using Path = typename MainLoop<OpenList, ClosedList>::Path;
};

template<int cost>
struct Test
{
	static constexpr int Cost = cost;
};

template<typename Left, typename Right>
struct FalseCondition
{
	static constexpr bool Value = false;
};

#define S 'S'
#define E 'E'

	using map = MapFromValue<8, char,
		S, 0, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 1, 0,
		0, 0, 0, 0, 0, 0, 1, 0,
		0, 0, E, 1, 1, 0, 1, 0,
		0, 0, 0, 1, 0, 0, 1, 0,
		0, 0, 0, 1, 0, 0, 1, 0,
		0, 1, 1, 1, 0, 0, 1, 0,
		0, 0, 0, 0, 0, 0, 0, 0
		>; 

#undef S
#undef E

/////////////////////////////////////////
//     OPTIONAL TESTS
////////////////////////////////////////

#define static_assert(...) static_assert(__VA_ARGS__, "")

static_assert(map::getFromPos<Vec<6, 0>> == 0);
static_assert(map::getFromPos<Vec<3, 4>> == 1);

static_assert(map::get<0> == 'S');
static_assert(map::getFromPos<Vec<0, 0>> == 'S');
static_assert(map::getFromPos<Vec<2, 3>> == 'E');

static_assert(map::find<'S'> == 0); // Start
static_assert(map::find<'E'> == 3*8 + 2); // End

static_assert(Vec<1, 2>::Index<8> == 17);
static_assert(std::is_same<FromIndex<8, 17>, Vec<1, 2>>::value);
static_assert(FromIndex<8, 17>::Index<8> == 17);

using TestVec1 = VectorFromValue<int, 0, 1, 2, 3, 4, 5>;
using TestVec2 = TestVec1::PushFront<6>;

static_assert(TestVec1::Get<0>::Value == 0);
static_assert(TestVec1::Get<1>::Value == 1);
static_assert(TestVec1::Get<5>::Value == 5);
static_assert(TestVec1::remove<2>::get<3> == 4);

static_assert(TestVec2::Get<0>::Value == 6);
static_assert(TestVec2::PopFront::value == 6);
static_assert(TestVec2::PopFront::vector::Size == TestVec1::Size);

using TestVec3 = Vector<Vec<0, 0>, Vec<1, 1>, Vec<2, 2>>;

static_assert(std::is_same<TestVec3::get<0>, Vec<0, 0>>::value);
static_assert(std::is_same<TestVec3::get<1>, Vec<1, 1>>::value);
static_assert(std::is_same<TestVec3::get<2>, Vec<2, 2>>::value);
static_assert(std::is_same<TestVec3::get<0>, Vec<2, 2>>::value == false);

using TestVector4 = Vector<Test<0>, Test<-1>, Test<1>, Test<8>, Test<-1>>;
using LeastNode = FindLeast<TestVector4>;
static_assert(LeastNode::Index == 1);
static_assert(LeastNode::Type::Cost == -1);

using TestVector5 = Vector<Test<0>, Test<-1>, Test<1>, Test<8>, Test<-1>>;
using LeastNode2 = FindLeast<TestVector5, Test<9>, -1, FalseCondition>;

static_assert(LeastNode2::Index == -1);
static_assert(std::is_same<LeastNode2::Type, Test<9>>::value);

using astar = AStar<map>;

using Node1 = typename astar::StartNode;
using Node2 = Node<Vec<7, 7>::Index<8>, 0, 0>;
using Node3 = Node<Vec<0, 1>::Index<8>, 0, 0>;
using Node4 = Node<Vec<1, 1>::Index<8>, 0, 0>;

static_assert(astar::template BuildNeighbors<Node1>::Type::Size == 2);
static_assert(astar::template BuildNeighbors<Node2>::Type::Size == 2);
static_assert(astar::template BuildNeighbors<Node3>::Type::Size == 3);
static_assert(astar::template BuildNeighbors<Node4>::Type::Size == 4);

/////////////////////////////////////////
//             TESTS END
////////////////////////////////////////

int main()
{
	using Path = astar::Path;

	using Test = Path::Test;

	return Path::Size;
}
