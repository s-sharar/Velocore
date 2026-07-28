"use client"

import { useUser } from "@/hooks/use-user"
import { Card, CardContent, CardHeader, CardTitle } from "@/components/ui/card"
import { Button } from "@/components/ui/button"

export default function ProfilePage() {
  const { user, loading, setUser } = useUser()

  const logout = async () => {
    await fetch("/api/auth/logout", { method: "POST" })
    setUser(null)
    location.href = "/login"
  }

  if (loading) return <div className="p-6">Loading...</div>
  if (!user) return <div className="p-6">Not signed in.</div>

  return (
    <div className="max-w-lg mx-auto mt-10">
      <Card>
        <CardHeader>
          <CardTitle>Your Profile</CardTitle>
        </CardHeader>
        <CardContent className="space-y-3">
          <div>
            <div className="text-sm text-gray-500">Name</div>
            <div className="font-medium">{user.name || "Unnamed"}</div>
          </div>
          <div>
            <div className="text-sm text-gray-500">Email</div>
            <div className="font-mono">{user.email}</div>
          </div>
          <Button onClick={logout} variant="outline">Sign out</Button>
        </CardContent>
      </Card>
    </div>
  )
}



