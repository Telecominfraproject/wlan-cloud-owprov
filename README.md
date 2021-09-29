# OpenWiFi Provisioning

## Build from source.
You need:
- https://github.com/pboettch/json-schema-validator.git
- https://github.com/nlohmann/json.git

build and install them.

## Root entity
It's UUID value is 0000-0000-0000. Its parent entity must be empty.

## Entity
### Creation rules
- You must set the parent of an entity.
- The only properties you may set at creation are:
  - name
  - description
  - notes
  - parent

### Modification rules
You may modify the following fields in the POST
- name
- description
- notes

### Delete
- Children must be empty

## Inventory Tags
### Creation rules
- Entity must point to an existing non-root entity
- If you associate a venue, it must exist.
- You must use an existing device type. Device type cannot be empty.
- Name, description, notes are allowed.

### Modification rules
- You can modify the device type to another valid one.

## Venue
### Creation rules
- If you include an entity, the parent must bot be set
- if you include a parent, the entity must not be set
- You cannot have children upon creation.
- You may include an array of devices UUIDs
- Topology and design cannot be set

### Modification rules



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

