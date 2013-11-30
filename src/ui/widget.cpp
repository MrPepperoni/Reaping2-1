#include "i_ui.h"

Widget::WidgetIterator & Widget::WidgetIterator::operator++()
{
	if(mThis->mFirstChild)
	{
		mThis=mThis->mFirstChild;
		++mLevel;
	}
	else if(mThis->mNext)
	{
		mThis=mThis->mNext;
	}
	else
	{
		Widget const* Next=mThis;
		while(Next&&!Next->mNext&&mLevel)
		{
			Next=Next->mParent;
			--mLevel;
		}
		mThis=Next->mNext;
	}
	return *this;
}

void Widget::AddChild(Widget* Child)
{
	assert(!Child->mPrev&&!Child->mParent&&!Child->mNext);

	Widget* WhatWillBeNext=mFirstChild;
	while(WhatWillBeNext&&WhatWillBeNext->mZOrder<Child->mZOrder)WhatWillBeNext=WhatWillBeNext->mNext;

	if(WhatWillBeNext)
	{
		Child->mPrev=WhatWillBeNext->mPrev;
		if(WhatWillBeNext->mPrev)
			WhatWillBeNext->mPrev->mNext=Child;
		Child->mNext=WhatWillBeNext;
		WhatWillBeNext->mPrev=Child;
		if(WhatWillBeNext==mFirstChild)
			mFirstChild=Child;
	}
	else
	{
		if(!mFirstChild)
			mFirstChild=Child;
		mLastChild=Child;
	}
	Child->mParent=this;
	Child->UpdateDimensions();
	Child->UpdateChildrenDimensions();
}

Widget::Widget(int32_t Id)
: mTypeId(Id)
, mZOrder(0)
, mParent(NULL)
, mNext(NULL)
, mPrev(NULL)
, mFirstChild(NULL)
, mLastChild(NULL)
, mDimSet(false)
, mRelativeDimensions(0,0,1,1)
{
	operator()(PT_Highlight)=0;
}

Widget::~Widget()
{
	Widget* What=mFirstChild;
	while(What)
	{
		Widget* Next=What->mNext;
		delete What;
		What=Next;
	}
	if(mNext)
	{
		mNext->mPrev=mPrev;
	}
	if(mPrev)
	{
		mPrev->mNext=mNext;
	}
	if(mParent)
	{
		if(mParent->mFirstChild==this)
			mParent->mFirstChild=mNext;
		if(mParent->mLastChild==this)
			mParent->mLastChild=mPrev;
	}
}

glm::vec4 const& Widget::GetDimensions() const
{
	return mDimensions;
}

Widget::const_iterator Widget::begin() const
{
	return WidgetIterator(this);
}

Widget::const_iterator Widget::end() const
{
	return WidgetIterator();
}

void Widget::UpdateDimensions()
{
	UpdateSelfDimensions();
	UpdateChildrenDimensions();
}

void Widget::UpdateSelfDimensions()
{
	if(!mParent||!mParent->mDimSet) return;
	mDimSet=true;
	glm::vec4 const& ParentDim=mParent->GetDimensions();
	double const X=ParentDim.x;
	double const Y=ParentDim.y;
	double const Width=ParentDim.z;
	double const Height=ParentDim.w;
	mDimensions=glm::vec4(	X+mRelativeDimensions.x*Width,
							Y+mRelativeDimensions.y*Height,
							mRelativeDimensions.z*Width,
							mRelativeDimensions.w*Height);
}

void Widget::UpdateChildrenDimensions()
{
	if(!mParent||!mParent->mDimSet) return;
	for(Widget* i=mFirstChild;i;i=i->mNext)
		i->UpdateDimensions();
}

Widget* Widget::GetHit( const glm::vec2& Pos )
{
	if(!IsInside(Pos))return NULL;
	for(Widget* i=mLastChild;i;i=i->mPrev)
	{
		Widget* Wdg=i->GetHit(Pos);
		if(Wdg) return Wdg;
	}
	return mProperties(PT_Enabled).Value.ToInt?this:NULL;
}

bool Widget::IsInside( const glm::vec2& Pos ) const
{
	return mDimSet&&
			Pos.x>=mDimensions.x&&
			Pos.x<=mDimensions.x+mDimensions.z&&
			Pos.y>=mDimensions.y&&
			Pos.y<=mDimensions.y+mDimensions.w;
}

void Widget::SetRelativeDimensions( glm::vec4 const& Dim )
{
	mRelativeDimensions=Dim;
	UpdateDimensions();
}

bool Widget::AreDimensionsSet() const
{
	return mDimSet;
}

Widget* Widget::GetNext() const
{
	return mNext;
}

Widget::Prop const& Widget::operator()( PropertyType Property ) const
{
	return mProperties(Property);
}

Widget::Prop& Widget::operator()( PropertyType Property )
{
	return mProperties.Mutable(Property);
}

int32_t Widget::ParseColor(Json::Value& Color,int32_t Default)
{
	std::string s;
	int32_t i;
	if(Json::GetInt(Color,i))
		return i;
	else if(Json::GetStr(Color,s)&&(i=strtoul(s.c_str(),NULL,16)))
	{
		size_t const h=s.find('x');
		if(s.size()<=6&&h==std::string::npos||
			s.size()<=8&&h==2)
		{
			// make it rgba
			i=(i<<8)|0xff;
		}
	}
	else
		i=Default;
	return i;
}

void Widget::Init( Json::Value& Descriptor )
{
	double d;
	mRelativeDimensions.x=Json::GetDouble(Descriptor["x"],d)?d:0;
	mRelativeDimensions.y=Json::GetDouble(Descriptor["y"],d)?d:0;
	mRelativeDimensions.z=Json::GetDouble(Descriptor["w"],d)?d:0;
	mRelativeDimensions.w=Json::GetDouble(Descriptor["h"],d)?d:0;
	static const int32_t DefaultColor=0x0000fa77;
	int32_t i=ParseColor(Descriptor["color"],DefaultColor);
	operator()(PT_Color)=i;
	operator()(PT_HighlightColor)=ParseColor(Descriptor["highlight_color"],i);
	operator()(PT_Enabled)=Json::GetInt(Descriptor["enabled"],i)?i:0;
	operator()(PT_Visible)=Json::GetInt(Descriptor["visible"],i)?i:0;
	Json::Value& Children=Descriptor["children"];
	if(!Children.isArray()||!Children.size())return;
	WidgetFactory& Fact(WidgetFactory::Get());
	for(Json::Value::iterator i=Children.begin(),e=Children.end();i!=e;++i)
	{
		std::string Str;
		if(!Json::GetStr((*i)["type"],Str))continue;
		Widget* wdg=Fact(Str,*i);
		AddChild(wdg);
	}
}

int32_t Widget::GetId() const
{
	return mTypeId;
}

Widget::Prop::Prop()
:Type(T_Null)
{
	Value.ToInt=0;
}

Widget::Prop::Prop( int32_t IntVal )
:Type(T_Int)
{
	Value.ToInt=IntVal;
}

Widget::Prop::Prop( double DoubleVal )
:Type(T_Double)
{
	Value.ToDouble=DoubleVal;
}

Widget::Prop::Prop( const std::string& StrVal )
:Type(T_Str)
{
	Init(StrVal);
}

Widget::Prop::Prop( Prop const& Other )
:Type(Other.Type)
{
	if(Type==T_Str)
		Init(std::string(Other.Value.ToStr));
}

Widget::Prop::~Prop()
{
	if(Type==T_Str)
		delete Value.ToStr;
}

Widget::Prop::operator char const*() const
{
	assert(Type==T_Str);
	return (Type==T_Str)?Value.ToStr:NULL;
}

Widget::Prop::operator int32_t() const
{
	assert(Type==T_Int||Type==T_Double);
	return (Type==T_Int)?Value.ToInt:(Type==T_Double?int32_t(Value.ToDouble):0);
}

Widget::Prop::operator double() const
{
	assert(Type==T_Int||Type==T_Double);
	return (Type==T_Double)?Value.ToDouble:(Type==T_Int?double(Value.ToInt):0.0);
}

Widget::Prop& Widget::Prop::operator=( std::string const& Str )
{
	Cleanup();
	Type=T_Str;
	Init(Str);
	return *this;
}

Widget::Prop& Widget::Prop::operator=( char const* Str )
{
	Cleanup();
	Type=T_Str;
	Init(std::string(Str));
	return *this;
}

Widget::Prop& Widget::Prop::operator=( int32_t I )
{
	Cleanup();
	Type=T_Int;
	Value.ToInt=I;
	return *this;
}

Widget::Prop& Widget::Prop::operator=( double D )
{
	Cleanup();
	Type=T_Double;
	Value.ToDouble=D;
	return *this;
}

void Widget::Prop::Init( std::string const& StrVal )
{
	Type=T_Str;
	if(StrVal.empty())
	{
		Value.ToStr=new char(0);
		return;
	}
	const size_t Size=StrVal.size();
	char* Buf=new char[Size+1];
	memset(Buf,0,Size+1);
	memcpy(Buf,StrVal.c_str(),Size);
	Value.ToStr=Buf;
}

void Widget::Prop::Cleanup()
{
	if(Type==T_Str)
		delete Value.ToStr;
	Type=T_Null;
}

Widget::Prop::operator std::string() const
{
	return std::string(operator char const*());
}

Widget::PropertyRepo_t::PropertyRepo_t()
:RepoBase(DefaultProperty)
{

}

Widget::Prop& Widget::PropertyRepo_t::Mutable( PropertyType Property )
{
	int32_t Id(Property);
	return mElements[Id];
}

Widget::Prop Widget::PropertyRepo_t::DefaultProperty=Widget::Prop();

