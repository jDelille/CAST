import React from "react";
import Button from "../reusable/button/Button"

  type NavbarProps = {
    isAuthPage?: boolean;
  };

  const Navbar: React.FC<NavbarProps> = ({isAuthPage}) => {

    const handleClick = () => {
      console.log("Clicked...");
    }

    return (
      <nav className="navbar">
        <div className="logo">
          <img src="" alt="" />
        </div>
        <Button text={isAuthPage ? "Login" : "Signup"} onClick={handleClick} />
      </nav>
    );
  }

  export default Navbar;

