# SimpleSocialMedia
A client/server social media program. The server accepts connections from clients through UDP socket connections. This is a simple implementation, there is no user authentication, so clients login by simply entering an int, if the int value does not have an active connection, then the client has logged on as the user identified by the int entered by the client.
## Options
Once a client connects to the server, the client can
- Login: client can login to the server
- Post: client can enter 140 character message to be posted to the server using '#' to create tags
- Delete: client can delete posts they've made
- Search: client can search posts based on their tags
- Follow: client can follow other clients connected to the server
- Request: client can request a list of all the posts of the other clients they follow
- Unfollow: client can unfollow clients they are following
- Logout: client can 'logout' and disconnect from the server
