# OpenWiFi Provisioning

## Root entity
It's UUID value is 0000-0000-0000. Its parent entity must be empty.

## Entity
You must set the parent of an entity.

## Venue
When creating a venue, the top venue must have its entity property set to the owning entity, and its parent property empty. 
For all sub venues, their entity must be set to empty and its parent entity must be set to the venue above it.

## Management policy

```json
{
    "default" : [],
    "acls" : [
        {
            "roles" : [ uuid1, uuid2, uuid3 ],
            "access" : [ READ, WRITE, ... ]
        } ,
        {
            "roles" : [ ... ],
            "access" : [ ... ]
        }
  ]
}
```

## Management roles
Management roles can be created using UUIDs from the SEC service. SEC service may ask prov if deleting a user 
is OK. PROV should answer with username in use or something like this. 

Management roles are created by adding UUIDs into a group. Then that UUID may be used in any management 
policy.

Management roles must have a quick way to evaluate all the roles a user has. This is important for 
speed. Roles ddo not use subscribers.

So read all the roles, cross ref all the users sp you can apply access rules against a resource very quickly.

If a user is part of 2 roles, then the access will be agregate. if NONE is found, then NONE wins.

