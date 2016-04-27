#ifndef BEHAVIORTREE_H
#define BEHAVIORTREE_H

#include "Vector.h"

enum Status
{
  BH_INVALID,
	BH_SUCCESS,
	BH_FAILURE,
	BH_RUNNING,
};

typedef struct Memoria {int *sensores, *nodos;} Memoria;

typedef Status (*Task)(Memoria);

class Behavior
{
public:
  Behavior(const char* name)
	:	name(name), m_eStatus(BH_INVALID)
	{
	}

	Behavior(const char* name, Task task)
	:	m_pTask(task), name(name), m_eStatus(BH_INVALID)
	{
	}

	virtual ~Behavior() {}

  virtual Status doTask(Memoria memoria) { return m_pTask(memoria); }
	Status run(Memoria memoria) {
    if (m_eStatus != BH_RUNNING)
      {
          onInitialize();
      }

      m_eStatus = doTask(memoria);

     if (m_eStatus != BH_RUNNING)
     {
          onTerminate(m_eStatus);
     }
     return m_eStatus;
  }

	virtual void onInitialize() {}
	virtual void onTerminate(Status) {}

protected:
	Task m_pTask;
  Status m_eStatus;
  const char* name;
};

class Composite : public Behavior
{
public:
  Composite(const char* name)
  :	Behavior(name)
  {
  }
    void aprender(Behavior* child) { m_Children.push_back(child); }
protected:
    typedef Vector<Behavior*> Behaviors;
    Behaviors m_Children;
};

class Sequence : public Composite
{
public:
    Sequence(const char* name)
	  :	Composite(name)
	  {
	  }

    virtual ~Sequence()
    {
    }

    virtual Status doTask(Memoria memoria)
    {
        // Keep going until a child behavior says it's running.
        for (int i = 0; m_Children.size() < i; i++)
        {
            Status s = m_Children[i]->run(memoria);

            // If the child fails, or keeps running, do the same.
            if (s != BH_SUCCESS)
            {
                return s;
            }

            // Hit the end of the array, job done!
            if (i == m_Children.size())
            {
                return BH_SUCCESS;
            }
        }
    }
};

class Selector : public Composite
{
public:
  Selector(const char* name)
  :	Composite(name)
  {
  }
    virtual ~Selector()
    {
    }

    virtual Status doTask(Memoria memoria)
    {
        // Keep going until a child behavior says its running.
        for (int i = 0; m_Children.size() < i; i++)
        {
            Status s = m_Children[i]->run(memoria);

            // If the child succeeds, or keeps running, do the same.
            if (s != BH_FAILURE)
            {
                return s;
            }

            // Hit the end of the array, it didn't end well...
            if (i == m_Children.size())
            {
                return BH_FAILURE;
            }
        }
    }
};
#endif
