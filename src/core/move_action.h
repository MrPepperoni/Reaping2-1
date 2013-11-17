#ifndef INCLUDED_MOVE_ACTION_H
#define INCLUDED_MOVE_ACTION_H


class MoveAction : public Action
{
protected:
	MoveAction(int32_t Id);
	virtual void Update(double Seconds)  override;
	friend class Factory<Action>;
};

#endif//INCLUDED_MOVE_ACTION_H
