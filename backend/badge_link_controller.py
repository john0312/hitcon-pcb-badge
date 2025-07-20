from config import Config
from database import db
from typing import Optional

config = Config("config.yaml")

class BadgeLinkController:
    @staticmethod
    async def translate_uid_to_user(uid: str) -> Optional[int]:
        """
        Translate attendee UID to user ID.
        Return None if the attendee is not yet linked to with a badge.
        """
        # TODO: Implement the logic to translate UID to badge username.
        pass


    @staticmethod
    async def get_uid_with_token(token: str) -> Optional[str]:
        # TODO: Implement the logic for retrieving UID with HITCON token
        # return None if the token is invalid
        return token # TODO: TO BE REPLACED


    @staticmethod
    async def link_badge_with_attendee(uid: str, badge_user: int):
        """
        Link an attendee to a badge.
        Raise exception if the badge is already linked to another attendee.
        """
        # TODO: Implement the logic for linking attendee to badge with token
        pass
