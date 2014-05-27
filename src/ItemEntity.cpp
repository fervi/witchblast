#include "ItemEntity.h"
#include "sfml_game/ImageManager.h"
#include "sfml_game/SoundManager.h"
#include "sfml_game/SpriteEntity.h"
#include "Constants.h"
#include "MagnetEntity.h"
#include "WitchBlastGame.h"
#include <iostream>
#include <sstream>



ItemEntity::ItemEntity(enumItemType itemType, float x, float y)
    : CollidingSpriteEntity(ImageManager::getImageManager()->getImage(itemType >= FirstEquipItem ? IMAGE_ITEMS_EQUIP : IMAGE_ITEMS), x, y, ITEM_WIDTH, ITEM_HEIGHT)
{
  type = ENTITY_ITEM;
  this->itemType = itemType;
  frame = itemType;
  if (itemType >= FirstEquipItem) frame = itemType - FirstEquipItem;
  isMerchandise = false;
  imagesProLine = 10;
  setMap(game().getCurrentMap(), TILE_WIDTH, TILE_HEIGHT, OFFSET_X, OFFSET_Y);
  isBeating = false;
}

void ItemEntity::setMerchandise(bool isMerchandise)
{
  this->isMerchandise = isMerchandise;
}
bool ItemEntity::getMerchandise()
{
  return isMerchandise;
}

int ItemEntity::getPrice()
{
  return (items[itemType].price);
}

void ItemEntity::animate(float delay)
{
  z = y + height;
  CollidingSpriteEntity::animate(delay);
  if (age > 0.7f) testSpriteCollisions();

  if (isBeating)
  {
    timer -= delay;
    if (timer <= 0.0f)
    {
      timer = HEART_BEAT_DELAY;
      SoundManager::getSoundManager()->playSound(SOUND_HEART);
    }
    float sc;
    if (timer > HEART_BEAT_DELAY - 0.25f)
    {
      sc = timer - HEART_BEAT_DELAY + 1.25f;
    }
    else
      sc = 1.0f;
    sprite.setScale(sc, sc);
  }
  if (itemType == itemBossHeart && !isBeating && game().getCurrentMap()->isCleared())
  {
    // start beating
    isBeating = true;
    timer = HEART_BEAT_DELAY;
  }
}

void ItemEntity::render(sf::RenderTarget* app)
{
  // shadow
  sprite.setTextureRect(sf::IntRect(9 * width, height, width, height));
  app->draw(sprite);

  // price
  if (isMerchandise)
  {
    std::ostringstream oss;
    oss << getPrice() << " $";
    game().Write(oss.str(), 16, x, y + 18.0f, ALIGN_CENTER, sf::Color(255, 255, 255), app);
  }

  CollidingSpriteEntity::render(app);
}

void ItemEntity::calculateBB()
{
  boundingBox.left = (int)x - width / 2;
  boundingBox.width = width;
  boundingBox.top = (int)y - height / 2;
  boundingBox.height =  height;
}

void ItemEntity::dying()
{
  isDying = true;
}

void ItemEntity::readCollidingEntity(CollidingSpriteEntity* entity)
{
  if (itemType == itemBossHeart && !game().getCurrentMap()->isCleared()) return;
    PlayerEntity* playerEntity = dynamic_cast<PlayerEntity*>(entity);

    if (collideWithEntity(entity))
    {
      if (playerEntity != NULL && !playerEntity->isDead())
      {
        if (isMerchandise == false || playerEntity->getGold() >= getPrice())
        {
          playerEntity->acquireItem(itemType);

          if (isMerchandise) playerEntity->pay(getPrice());

          dying();

          if (items[itemType].generatesStance)
          {

            game().showArtefactDescription(itemType);
            SpriteEntity* spriteItem = new SpriteEntity(
                            image,
                            playerEntity->getX(), playerEntity->getY() - 60.0f, ITEM_WIDTH, ITEM_HEIGHT);
            spriteItem->setFrame(frame);
            spriteItem->setImagesProLine(10);
            spriteItem->setZ(z);
            spriteItem->setLifetime(ACQUIRE_DELAY);

            SpriteEntity* spriteStar = new SpriteEntity(
                           ImageManager::getImageManager()->getImage(IMAGE_STAR),
                            playerEntity->getX(), playerEntity->getY() - 60.0f);
            spriteStar->setScale(4.0f, 4.0f);
            spriteStar->setZ(z-1.0f);
            spriteStar->setLifetime(ACQUIRE_DELAY);
            spriteStar->setSpin(50.0f);
          }
          else
            new MagnetEntity(x, y, playerEntity, itemType);
        }
      }
    }
}

void ItemEntity::collideMapRight()
{
    velocity.x = -velocity.x * 0.66f;
}

void ItemEntity::collideMapLeft()
{
    velocity.x = -velocity.x * 0.66f;
}

void ItemEntity::collideMapTop()
{
    velocity.y = -velocity.y * 0.66f;
}

void ItemEntity::collideMapBottom()
{
    velocity.y = -velocity.y * 0.66f;
}
