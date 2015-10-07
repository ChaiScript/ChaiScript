#include <chaiscript/chaiscript.hpp>
#include <chaiscript/chaiscript_stdlib.hpp>

class Entity
{
  public:
    int width;
    int height;
    int x;
    int y;
    std::string name;

    std::function<void (Entity &)> updater;

    Entity(const int t_width, const int t_height, const int t_x, const int t_y, std::string t_name)
      : width(t_width), height(t_height), x(t_x), y(t_y), name(std::move(t_name))
    {
    }
};

class Factory
{
  public:
    // we may as well pass the parameters for the entity to the factory method, this does the initialization
    // in one step.
    Entity *make_entity(const int width, const int height, const int x, const int y, const std::string &name)
    {
      auto entity = entities.insert({name, Entity{width, height, x, y, name}});
      return &(entity.first->second);
    }

    Entity *get_entity(const std::string &name)
    {
      return &entities.at(name);
    }


    // loop over all entities and all their updater function (if it exists)
    void update_entities()
    {
      for (auto &entity : entities)
      {
        if (entity.second.updater) {
          entity.second.updater(entity.second);
        }
      }
    }


  private:
    // we cannot store the entities in a std::vector if we want to return a pointer to them,
    // because a vector automatically resizing itself can invalidate the pointer that was returned.
    // using a map guarantees that the memory assigned to the entity will never change, plus
    // lets us easily look up an entity by name
    std::map<std::string, Entity> entities;
};

int main()
{
  chaiscript::ChaiScript chai(chaiscript::Std_Lib::library());

  chai.add(chaiscript::fun(&Entity::width), "width");
  chai.add(chaiscript::fun(&Entity::height), "height");
  chai.add(chaiscript::fun(&Entity::x), "x");
  chai.add(chaiscript::fun(&Entity::y), "y");
  chai.add(chaiscript::fun(&Entity::name), "name");
  chai.add(chaiscript::fun(&Entity::updater), "updater");
  chai.add(chaiscript::user_type<Entity>(), "Entity"); // this isn't strictly necessary but makes error messages nicer

  chai.add(chaiscript::fun(&Factory::make_entity), "make_entity");
  chai.add(chaiscript::fun(&Factory::get_entity), "get_entity");
  chai.add(chaiscript::fun(&Factory::update_entities), "update_entities");
  chai.add(chaiscript::user_type<Factory>(), "Factory"); // this isn't strictly necessary but makes error messages nicer


  Factory f;
  chai.add(chaiscript::var(&f), "f");

  std::string script = R""(
    f.make_entity(10,10,1,1,"entity1").updater = fun(e){ e.x += 1; e.y += 1 };
    f.make_entity(10,10,10,10,"entity2").updater = fun(e){ e.x += 2; e.y += 2 };
    f.make_entity(10,10,20,20,"entity3");

    print(f.get_entity("entity1").x == 1)
    print(f.get_entity("entity2").x == 10)
    print(f.get_entity("entity3").x == 20)

    f.update_entities(); // this runs the function objects we set in the previous lines
    // we should now see the updated values

    print(f.get_entity("entity1").x == 2)
    print(f.get_entity("entity2").x == 12)
    print(f.get_entity("entity3").x == 20) // this one has no updater, so it stays the same
    )"";


  chai.eval(script);



}


